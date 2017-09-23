#include "CLVideoFilter.hpp"


#include <QAbstractVideoBuffer>
#include <QAbstractVideoFilter>
#include <QImage>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QOpenGLFunctions_4_0_Core>


#ifdef Q_OS_LINUX
#include <QtPlatformHeaders/QGLXNativeContext>
#endif

#include <QFileInfo>


#include "private/qvideoframe_p.h"


/*
  Returns a QImage that wraps the given video frame.

  This is suitable only for QAbstractVideoBuffer::NoHandle frames with RGB (or BGR)
  data. YUV is not supported here.

  The QVideoFrame must be mapped and kept mapped as long as the wrapping QImage
  exists.

  As a convenience the function also supports frames with a handle type of
  QAbstractVideoBuffer::GLTextureHandle. This allows creating a system memory backed
  QVideoFrame containing the image data from an OpenGL texture. However, readback is a
  slow operation and may stall the GPU pipeline and should be avoided in production code.
*/
QImage imageWrapper(const QVideoFrame &frame)
{
#ifndef QT_NO_OPENGL
	if (frame.handleType() == QAbstractVideoBuffer::GLTextureHandle) {
		// Slow and inefficient path. Ideally what's on the GPU should remain on the GPU, instead of readbacks like this.
		QImage img(frame.width(), frame.height(), QImage::Format_RGBA8888);
		GLuint textureId = frame.handle().toUInt();
		QOpenGLContext *ctx = QOpenGLContext::currentContext();
		QOpenGLFunctions *f = ctx->functions();
		GLuint fbo;
		f->glGenFramebuffers(1, &fbo);
		GLuint prevFbo;
		f->glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint *) &prevFbo);
		f->glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		f->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0);
		f->glReadPixels(0, 0, frame.width(), frame.height(), GL_RGBA, GL_UNSIGNED_BYTE, img.bits());
		f->glBindFramebuffer(GL_FRAMEBUFFER, prevFbo);
		return img;
	} else
#endif // QT_NO_OPENGL
	{
		if (!frame.isReadable()) {
			qWarning("imageFromVideoFrame: No mapped image data available for read");
			return QImage();
		}

		QImage::Format fmt = QVideoFrame::imageFormatFromPixelFormat(frame.pixelFormat());
		if (fmt != QImage::Format_Invalid) {
			return QImage(frame.bits(), frame.width(), frame.height(), fmt);
		}

		qWarning("imageFromVideoFrame: No matching QImage format");
	}

	return QImage();
}

#ifndef QT_NO_OPENGL
class TextureBuffer : public QAbstractVideoBuffer
{
public:
	TextureBuffer(uint id) : QAbstractVideoBuffer(GLTextureHandle), m_id(id) { }
	MapMode mapMode() const
	{
		return NotMapped;
	}
	uchar *map(MapMode, int *, int *)
	{
		return 0;
	}
	void unmap() { }
	QVariant handle() const
	{
		return QVariant::fromValue<uint>(m_id);
	}

private:
	GLuint m_id;
};
#endif // QT_NO_OPENGL

/*
  Creates and returns a new video frame wrapping the OpenGL texture textureId. The size
  must be passed in size, together with the format of the underlying image data in
  format. When the texture originates from a QImage, use
  QVideoFrame::imageFormatFromPixelFormat() to get a suitable format. Ownership is not
  altered, the new QVideoFrame will not destroy the texture.
*/
QVideoFrame frameFromTexture(uint textureId, const QSize &size, QVideoFrame::PixelFormat format)
{
#ifndef QT_NO_OPENGL
	return QVideoFrame(new TextureBuffer(textureId), size, format);
#else
	return QVideoFrame();
#endif // QT_NO_OPENGL
}



static const char *openclSrc =
	"__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;\n"
	"__kernel void Emboss(__read_only image2d_t imgIn, __write_only image2d_t imgOut, float factor) {\n"
	"    const int2 pos = { get_global_id(0), get_global_id(1) };\n"
	"    float4 diff = read_imagef(imgIn, sampler, pos + (int2)(1,1)) - read_imagef(imgIn, sampler, pos - (int2)(1,1));\n"
	"    float color = (diff.x + diff.y + diff.z) / factor + 0.5f;\n"
	"    write_imagef(imgOut, pos, (float4)(color, color, color, 1.0f));\n"
	"}\n";



CLVideoFilter::CLVideoFilter() :
	m_tempTexture(0),
	m_outTexture(0),
	m_lastInputTexture(0),
	m_clContext(0),
	m_clQueue(0),
	m_clProgram(0),
	m_clKernel(0)
  ,isInited(false)
{

}



CLVideoFilter::~CLVideoFilter()
{
	releaseTextures();
	if (m_clKernel) {
		clReleaseKernel(m_clKernel);
	}
	if (m_clProgram) {
		clReleaseProgram(m_clProgram);
	}
	if (m_clQueue) {
		clReleaseCommandQueue(m_clQueue);
	}
	if (m_clContext) {
		clReleaseContext(m_clContext);
	}
}



void CLVideoFilter::init()
{
	if(isInited){
		return;
	}
	isInited=true;
	m_clImage[0] = m_clImage[1] = 0;

	// Set up OpenCL.
	//QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
	QOpenGLFunctions_4_0_Core *f=glFunctions();
	cl_uint n;
	cl_int err = clGetPlatformIDs(0, 0, &n);
	if (err != CL_SUCCESS) {
		qWarning("Failed to get platform ID count (error %d)", err);
		if (err == -1001) {
			qDebug("Could not find OpenCL implementation. ICD missing?"
#ifdef Q_OS_LINUX
				   " Check /etc/OpenCL/vendors."
#endif
				  );
		}
		return;
	}
	if (n == 0) {
		qWarning("No OpenCL platform found");
		return;
	}
	QVector<cl_platform_id> platformIds;
	platformIds.resize(n);
	if (clGetPlatformIDs(n, platformIds.data(), 0) != CL_SUCCESS) {
		qWarning("Failed to get platform IDs");
		return;
	}
	cl_platform_id platform = platformIds[0];
	const char *vendor = (const char *) f->glGetString(GL_VENDOR);
	qDebug("GL_VENDOR: %s", vendor);
	const bool isNV = vendor && strstr(vendor, "NVIDIA");
	const bool isIntel = vendor && strstr(vendor, "Intel");
	const bool isAMD = vendor && strstr(vendor, "ATI");
	qDebug("Found %u OpenCL platforms:", n);
	for (cl_uint i = 0; i < n; ++i) {
		QByteArray name;
		name.resize(1024);
		clGetPlatformInfo(platformIds[i], CL_PLATFORM_NAME, name.size(), name.data(), 0);
		qDebug("Platform %p: %s", platformIds[i], name.constData());
		// Running with an OpenCL platform without GPU support is not going
		// to cut it. In practice we want the platform for the GPU which we
		// are using with OpenGL.
		if (isNV && name.contains(QByteArrayLiteral("NVIDIA"))) {
			platform = platformIds[i];
		} else if (isIntel && name.contains(QByteArrayLiteral("Intel"))) {
			platform = platformIds[i];
		} else if (isAMD && name.contains(QByteArrayLiteral("AMD"))) {
			platform = platformIds[i];
		}
	}
	qDebug("Using platform %p", platform);

	// Set up the context with OpenCL/OpenGL interop.
#if defined (Q_OS_OSX)
	cl_context_properties contextProps[] = { CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE,
											 (cl_context_properties) CGLGetShareGroup(CGLGetCurrentContext()),
											 0
										   };
#elif defined(Q_OS_WIN)
	cl_context_properties contextProps[] = { CL_CONTEXT_PLATFORM, (cl_context_properties) platform,
											 CL_GL_CONTEXT_KHR, (cl_context_properties) wglGetCurrentContext(),
											 CL_WGL_HDC_KHR, (cl_context_properties) wglGetCurrentDC(),
											 0
										   };
#elif defined(Q_OS_LINUX)
	QVariant nativeGLXHandle = QOpenGLContext::currentContext()->nativeHandle();
	QGLXNativeContext nativeGLXContext;
	if (!nativeGLXHandle.isNull() && nativeGLXHandle.canConvert<QGLXNativeContext>()) {
		nativeGLXContext = nativeGLXHandle.value<QGLXNativeContext>();
	} else {
		qWarning("Failed to get the underlying GLX context from the current QOpenGLContext");
	}
	cl_context_properties contextProps[] = { CL_CONTEXT_PLATFORM, (cl_context_properties) platform,
											 CL_GL_CONTEXT_KHR, (cl_context_properties) nativeGLXContext.context(),
											 CL_GLX_DISPLAY_KHR, (cl_context_properties) glXGetCurrentDisplay(),
											 0
										   };
#endif

	m_clContext = clCreateContextFromType(contextProps, CL_DEVICE_TYPE_GPU, 0, 0, &err);
	if (!m_clContext) {
		qWarning("Failed to create OpenCL context: %d", err);
		return;
	}

	// Get the GPU device id
#if defined(Q_OS_OSX)
	// On OS X, get the "online" device/GPU. This is required for OpenCL/OpenGL context sharing.
	err = clGetGLContextInfoAPPLE(m_clContext, CGLGetCurrentContext(),
								  CL_CGL_DEVICE_FOR_CURRENT_VIRTUAL_SCREEN_APPLE,
								  sizeof(cl_device_id), &m_clDeviceId, 0);
	if (err != CL_SUCCESS) {
		qWarning("Failed to get OpenCL device for current screen: %d", err);
		return;
	}
#else
	clGetGLContextInfoKHR_fn getGLContextInfo = (clGetGLContextInfoKHR_fn) clGetExtensionFunctionAddress("clGetGLContextInfoKHR");
	if (!getGLContextInfo || getGLContextInfo(contextProps, CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR,
			sizeof(cl_device_id), &m_clDeviceId, 0) != CL_SUCCESS) {
		err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &m_clDeviceId, 0);
		if (err != CL_SUCCESS) {
			qWarning("Failed to get OpenCL device: %d", err);
			return;
		}
	}
#endif

	m_clQueue = clCreateCommandQueue(m_clContext, m_clDeviceId, 0, &err);
	if (!m_clQueue) {
		qWarning("Failed to create OpenCL command queue: %d", err);
		return;
	}
	// Build the program.
	m_clProgram = clCreateProgramWithSource(m_clContext, 1, &openclSrc, 0, &err);
	if (!m_clProgram) {
		qWarning("Failed to create OpenCL program: %d", err);
		return;
	}
	if (clBuildProgram(m_clProgram, 1, &m_clDeviceId, 0, 0, 0) != CL_SUCCESS) {
		qWarning("Failed to build OpenCL program");
		QByteArray log;
		log.resize(2048);
		clGetProgramBuildInfo(m_clProgram, m_clDeviceId, CL_PROGRAM_BUILD_LOG, log.size(), log.data(), 0);
		qDebug("Build log: %s", log.constData());
		return;
	}
	m_clKernel = clCreateKernel(m_clProgram, "Emboss", &err);
	if (!m_clKernel) {
		qWarning("Failed to create emboss OpenCL kernel: %d", err);
		return;
	}

}



void CLVideoFilter::releaseTextures()
{
	//QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
	QOpenGLFunctions_4_0_Core *f=glFunctions();
	if (m_tempTexture) {
		f->glDeleteTextures(1, &m_tempTexture);
	}
	if (m_outTexture) {
		f->glDeleteTextures(1, &m_outTexture);
	}
	m_tempTexture = m_outTexture = m_lastInputTexture = 0;
	if (m_clImage[0]) {
		clReleaseMemObject(m_clImage[0]);
	}
	if (m_clImage[1]) {
		clReleaseMemObject(m_clImage[1]);
	}
	m_clImage[0] = m_clImage[1] = 0;
}

uint CLVideoFilter::newTexture()
{
	//QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
	QOpenGLFunctions_4_0_Core *f=glFunctions();
	GLuint texture;
	f->glGenTextures(1, &texture);
	f->glBindTexture(GL_TEXTURE_2D, texture);
	f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_size.width(), m_size.height(),
					0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	return texture;
}



QOpenGLFunctions_4_0_Core *CLVideoFilter::glFunctions()
{
	auto ctx=QOpenGLContext::currentContext();
	QOpenGLFunctions_4_0_Core *f=nullptr;
	if(nullptr!=ctx) {
		f = static_cast<QOpenGLFunctions_4_0_Core *> (ctx->versionFunctions());
	}
	return f;
}

/*
void convertFromYUV(QVideoFrame *frame, QVideoFrame &out)
{

	const auto w=frame->width();
	const auto h=frame->height();
	const auto *bits=frame->bits(0);
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			const int xx = x >> 1;
			const int yy = y >> 1;
			const int Y = frame->bits(0)[y * frame->bytesPerLine(0) + x] - 16;
			const int U = frame->bits(1)[yy * frame->bytesPerLine(1) + xx] - 128;
			const int V = frame->bits(2)[yy * frame->bytesPerLine(2) + xx] - 128;
			const int r = qBound(0, (298 * Y + 409 * V + 128) >> 8, 255);
			const int g = qBound(0, (298 * Y - 100 * U - 208 * V + 128) >> 8, 255);
			const int b = qBound(0, (298 * Y + 516 * U + 128) >> 8, 255);

			image->setPixel(x, y, qRgb(r, g, b));
		}
	}

}
*/




QSharedPointer<QImage> CLVideoFilter::run(QVideoFrame *input)
{

	init();
	// This example supports RGB data only, either in system memory (typical with cameras on all
	// platforms) or as an OpenGL texture (e.g. video playback on OS X).
	// The latter is the fast path where everything happens on GPU. THe former involves a texture upload.
	QSharedPointer<QImage> out;
	if (!input->isValid()
			|| (input->handleType() != QAbstractVideoBuffer::NoHandle
				&& input->handleType() != QAbstractVideoBuffer::GLTextureHandle)) {
		qWarning("Invalid input format");
		return out;
	}


	QVideoFrame fr;
	if (input->pixelFormat() == QVideoFrame::Format_YUV420P
			|| input->pixelFormat() == QVideoFrame::Format_YV12) {
		//	qWarning("YUV data is maybe not supported");

		input->map(QAbstractVideoBuffer::ReadOnly);
		QImage im=qt_imageFromVideoFrame(*input);
		input->unmap();
		fr=QVideoFrame(im);
		input=&fr;


//			  return out;


	}

	if (m_size != input->size()) {
		releaseTextures();
		m_size = input->size();
	}

	// Create a texture from the image data.
	QOpenGLFunctions_4_0_Core *f = static_cast<QOpenGLFunctions_4_0_Core *> (QOpenGLContext::currentContext()->versionFunctions());
	GLuint texture;
	if (input->handleType() == QAbstractVideoBuffer::NoHandle) {
		// Upload.
		if (m_tempTexture) {
			f->glBindTexture(GL_TEXTURE_2D, m_tempTexture);
		} else {
			m_tempTexture = newTexture();
		}
		input->map(QAbstractVideoBuffer::ReadOnly);
		// glTexImage2D only once and use TexSubImage later on. This avoids the need
		// to recreate the CL image object on every frame.
		f->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_size.width(), m_size.height(),
						   GL_RGBA, GL_UNSIGNED_BYTE, input->bits());
		input->unmap();
		texture = m_tempTexture;
	} else {
		// Already an OpenGL texture.
		texture = input->handle().toUInt();
		f->glBindTexture(GL_TEXTURE_2D, texture);
		// Unlike on the other branch, the input texture may change, so m_clImage[0] may need to be recreated.
		if (m_lastInputTexture && m_lastInputTexture != texture && m_clImage[0]) {
			clReleaseMemObject(m_clImage[0]);
			m_clImage[0] = 0;
		}
		m_lastInputTexture = texture;
	}

	// OpenCL image objects cannot be read and written at the same time. So use
	// a separate texture for the result.
	if (!m_outTexture) {
		m_outTexture = newTexture();
	}

	// Create the image objects if not yet done.
	cl_int err;
	if (!m_clImage[0]) {
		m_clImage[0] = clCreateFromGLTexture2D(m_clContext, CL_MEM_READ_ONLY, GL_TEXTURE_2D, 0, texture, &err);
		if (!m_clImage[0]) {
			qWarning("Failed to create OpenGL image object from OpenGL texture: %d", err);
			return out;
		}
		cl_image_format fmt;
		if (clGetImageInfo(m_clImage[0], CL_IMAGE_FORMAT, sizeof(fmt), &fmt, 0) != CL_SUCCESS) {
			qWarning("Failed to query image format");
			return out;
		}
		if (fmt.image_channel_order != CL_RGBA) {
			qWarning("OpenCL image is not RGBA, expect errors");
		}
	}
	if (!m_clImage[1]) {
		m_clImage[1] = clCreateFromGLTexture2D(m_clContext, CL_MEM_WRITE_ONLY, GL_TEXTURE_2D, 0, m_outTexture, &err);
		if (!m_clImage[1]) {
			qWarning("Failed to create output OpenGL image object from OpenGL texture: %d", err);
			return out;
		}
	}


	// We are all set. Queue acquiring the image objects.
	f->glFinish();
	clEnqueueAcquireGLObjects(m_clQueue, 2, m_clImage, 0, 0, 0);

	// Set up the kernel arguments.
	clSetKernelArg(m_clKernel, 0, sizeof(cl_mem), &m_clImage[0]);
	clSetKernelArg(m_clKernel, 1, sizeof(cl_mem), &m_clImage[1]);
	// Accessing dynamic properties on the filter element is simple:
	cl_float factor = 5.0;
	clSetKernelArg(m_clKernel, 2, sizeof(cl_float), &factor);

	// And queue the kernel.
	const size_t workSize[] = { size_t(m_size.width()), size_t(m_size.height()) };
	err = clEnqueueNDRangeKernel(m_clQueue, m_clKernel, 2, 0, workSize, 0, 0, 0, 0);
	if (err != CL_SUCCESS) {
		qWarning("Failed to enqueue kernel: %d", err);
	}

	// Return the texture from our output image object.
	// We return a texture even when the original video frame had pixel data in system memory.
	// Qt Multimedia is smart enough to handle this. Once the data is on the GPU, it stays there. No readbacks, no copies.
	clEnqueueReleaseGLObjects(m_clQueue, 2, m_clImage, 0, 0, 0);
	clFinish(m_clQueue);


	out=QSharedPointer<QImage> (new QImage( input->width(), input->height(), QImage::Format_RGBA8888));


	f->initializeOpenGLFunctions();
	//(GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels)
	f->glGetTexImage
	(	(GLenum)m_outTexture
		, (GLint)0
		, (GLenum)GL_RGBA
		, (GLenum)GL_UNSIGNED_BYTE
		//,out.width() * out.height() * 4
		, (GLvoid *)out->bits());


//	return frameFromTexture(m_outTexture, m_size, input->pixelFormat());
	return out;
}
