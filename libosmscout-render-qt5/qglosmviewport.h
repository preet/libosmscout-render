#ifndef QGLOSMVIEWPORT_H
#define QGLOSMVIEWPORT_H

#include <QGLView>
#include <QGLBuilder>
#include <QGLCube>

class QGLOSMViewport : public QGLView
{
    Q_OBJECT
public:
    explicit QGLOSMViewport(QWindow *parent = 0);
    ~QGLOSMViewport();
signals:
    
public slots:

protected:
    void initializeGL(QGLPainter *glPainter);
    void paintGL(QGLPainter *glPainter);

private:
    QGLSceneNode * m_node_cube;
};

#endif // QGLOSMVIEWPORT_H
