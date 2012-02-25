#ifndef QGLOSMVIEWPORT_H
#define QGLOSMVIEWPORT_H

#include <QGLView>
#include <QGLBuilder>
#include <QGLSceneNode>
#include <QGLCube>
#include <QGLMaterial>
#include <QGraphicsRotation3D>
#include <QGLCylinder>

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
    QGLSceneNode * m_node_root;
    QGLSceneNode * m_node_origin;
    QGLCamera * m_camera;
};

#endif // QGLOSMVIEWPORT_H
