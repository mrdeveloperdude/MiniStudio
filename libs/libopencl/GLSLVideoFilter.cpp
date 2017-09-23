#include "GLSLVideoFilter.hpp"
#include <QOpenGLFunctions>


GLSLVideoFilterRunnable::GLSLVideoFilterRunnable(GLSLVideoFilter *filter)
	: m_filter(filter)
	, m_tempTexture(0)
	, m_outTexture(0)
	, m_lastInputTexture(0)
{

}

void GLSLVideoFilterRunnable::releaseTextures()
{
	QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
	if (0!=m_tempTexture) {
		f->glDeleteTextures(1, &m_tempTexture);
		m_tempTexture=0;
	}
	if (0!=m_outTexture) {
		f->glDeleteTextures(1, &m_outTexture);
		m_outTexture=0;
	}
	m_lastInputTexture = 0;
}


QVideoFrame GLSLVideoFilterRunnable::run(QVideoFrame *input, const QVideoSurfaceFormat &surfaceFormat, RunFlags flags)
{

	Q_UNUSED(input);
	Q_UNUSED(surfaceFormat);
	Q_UNUSED(flags);


	if (!input->isValid()
			|| (input->handleType() != QAbstractVideoBuffer::NoHandle
				&& input->handleType() != QAbstractVideoBuffer::GLTextureHandle)) {
		qWarning("Invalid input format");
		return *input;
	}

	if (input->pixelFormat() == QVideoFrame::Format_YUV420P
			|| input->pixelFormat() == QVideoFrame::Format_YV12) {
		qWarning("YUV data is not supported");
		return *input;
	}

	if (m_size != input->size()) {
		releaseTextures();
		m_size = input->size();
	}


	// Create a texture from the image data.
	QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
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
		m_lastInputTexture = texture;
	}

	QVideoFrame frame;
	return frame;

}

QVideoFilterRunnable *GLSLVideoFilter::createFilterRunnable()
{
	return new GLSLVideoFilterRunnable(this);
}
