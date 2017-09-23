#ifndef GLSLVIDEOFILTER_HPP
#define GLSLVIDEOFILTER_HPP




#include <QAbstractVideoFilter>
#include <QVideoFilterRunnable>



class GLSLVideoFilter;

class GLSLVideoFilterRunnable : public QVideoFilterRunnable
{
private:
	GLSLVideoFilter * m_filter;
	uint m_tempTexture;
	uint m_outTexture;
	uint m_lastInputTexture;
	QSize m_size;
private:

	void releaseTextures();
public:

	uint newTexture()
	{
		return -1;
	}

public:

	GLSLVideoFilterRunnable(GLSLVideoFilter *filter);

	QVideoFrame run(QVideoFrame *input, const QVideoSurfaceFormat &surfaceFormat, RunFlags flags);
};

class GLSLVideoFilter : public QAbstractVideoFilter
{
public:
	QVideoFilterRunnable *createFilterRunnable();
signals:
	void finished(QObject *result);
};



#endif // GLSLVIDEOFILTER_HPP
