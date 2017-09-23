#ifndef CLVIDEOFILTER_HPP
#define CLVIDEOFILTER_HPP

#include <QObject>
#include <QAbstractVideoFilter>
#include <QVideoFilterRunnable>
#include <QSharedPointer>

#ifdef Q_OS_OSX
#include <OpenCL/opencl.h>
#include <OpenGL/OpenGL.h>
#else
#include <CL/opencl.h>
#endif



class QOpenGLFunctions_4_0_Core;

class CLVideoFilter
{
private:
	QSize m_size;
	uint m_tempTexture;
	uint m_outTexture;
	uint m_lastInputTexture;
	cl_context m_clContext;
	cl_device_id m_clDeviceId;
	cl_mem m_clImage[2];
	cl_command_queue m_clQueue;
	cl_program m_clProgram;
	cl_kernel m_clKernel;
	bool isInited;
public:
	CLVideoFilter();
	~CLVideoFilter();
public:
	QSharedPointer<QImage> run(QVideoFrame *input);

	void init();

private:
	void releaseTextures();
	uint newTexture();


	QOpenGLFunctions_4_0_Core *glFunctions();


};

#endif // CLVIDEOFILTER_HPP
