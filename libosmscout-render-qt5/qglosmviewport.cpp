#include "qglosmviewport.h"

QGLOSMViewport::QGLOSMViewport(QWindow *parent) :
    QGLView(parent)
{

}

QGLOSMViewport::~QGLOSMViewport()
{   delete m_node_cube;   }

void QGLOSMViewport::initializeGL(QGLPainter *glPainter)
{
    QGLBuilder myBuilder;
    myBuilder << QGL::Faceted;
    myBuilder << QGLCube();

    m_node_cube = myBuilder.finalizedSceneNode();

    glPainter->setStandardEffect(QGL::LitMaterial);
    glPainter->setFaceColor(QGL::AllFaces, QColor(170,45,72));
}

void QGLOSMViewport::paintGL(QGLPainter *glPainter)
{
    m_node_cube->draw(glPainter);
}
