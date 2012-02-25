#include "qglosmviewport.h"

QGLOSMViewport::QGLOSMViewport(QWindow *parent) :
    QGLView(parent)
{
    // define camera
    m_camera = new QGLCamera;
    m_camera->setEye(QVector3D(0,10,30));
    m_camera->setFieldOfView(60);
    this->setCamera(m_camera);

    // create root node
    m_node_root = new QGLSceneNode;

    QGraphicsRotation3D * rotCS1 = new QGraphicsRotation3D;
    rotCS1->setAxis(QVector3D(1,0,0));
    rotCS1->setAngle(-90);

    QGraphicsRotation3D * rotCS2 = new QGraphicsRotation3D;
    rotCS2->setAxis(QVector3D(0,1,0));
    rotCS2->setAngle(-90);

    m_node_root->addTransform(rotCS1);
    m_node_root->addTransform(rotCS2);

        // create origin node
        m_node_origin = new QGLSceneNode(m_node_root);
        m_node_origin->setEffect(QGL::LitMaterial);

        // create axis node
        QGLBuilder buildOrigin;
        buildOrigin << QGLCylinder(1,1,10);

        QGLSceneNode * nodeAxis;
        nodeAxis = buildOrigin.finalizedSceneNode();

            // create z-axis node
            QGLSceneNode * nodeOriginZ = new QGLSceneNode(m_node_origin);
            QGLMaterial * matBlue = new QGLMaterial;
            matBlue->setColor(QColor(0,0,255,1));
            nodeOriginZ->addNode(nodeAxis);
            nodeOriginZ->setMaterial(matBlue);

            // create x-axis node
            QGLSceneNode * nodeOriginX = new QGLSceneNode(m_node_origin);
            QGLMaterial * matRed = new QGLMaterial;
            matRed->setColor(QColor(255,0,0,1));

            QGraphicsRotation3D * rotX = new QGraphicsRotation3D;
            rotX->setAxis(QVector3D(0,1,0));
            rotX->setAngle(90);

            nodeOriginX->addNode(nodeAxis);
            nodeOriginX->addTransform(rotX);
            nodeOriginX->setMaterial(matRed);

            // create y-axis node
            QGLSceneNode * nodeOriginY = new QGLSceneNode(m_node_origin);
            QGLMaterial * matGreen = new QGLMaterial;
            matGreen->setColor(QColor(0,255,0,1));

            QGraphicsRotation3D * rotY = new QGraphicsRotation3D;
            rotY->setAxis(QVector3D(1,0,0));
            rotY->setAngle(-90);

            nodeOriginY->addNode(nodeAxis);
            nodeOriginY->addTransform(rotY);
            nodeOriginY->setMaterial(matGreen);
}

QGLOSMViewport::~QGLOSMViewport()
{   delete m_node_root;   }

void QGLOSMViewport::initializeGL(QGLPainter *glPainter)
{

}

void QGLOSMViewport::paintGL(QGLPainter *glPainter)
{
    m_node_root->draw(glPainter);
}
