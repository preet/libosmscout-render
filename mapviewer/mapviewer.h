#ifndef MAPVIEWER_H
#define MAPVIEWER_H

#include <QMainWindow>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QStatusBar>
#include <QRadioButton>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QLabel>
#include <QFrame>
#include "viewport.h"

class MapViewer : public QWidget
{
    Q_OBJECT

public:
    explicit MapViewer(QWidget *parent = 0);
    ~MapViewer();

signals:
    void loadMap(QString const &,QString const &);
    void setCameraLLA(double camLat,double camLon,double camAlt);
    void setCamMode(int mode);

public slots:
    void onLoadButtonClicked();
    void onCamButtonClicked();
    void onCamModeRotate();
    void onCamModePan();
    void onCamModeZoom();

private:
    Viewport * m_viewport;

    QLabel * m_mapLabel;
    QLineEdit * m_mapLine;
    QPushButton * m_mapButton;

    QLabel * m_styleLabel;
    QLineEdit * m_styleLine;
    QPushButton * m_styleButton;

    QPushButton * m_loadButton;

    QLabel * m_camLatLabel;
    QLabel * m_camLonLabel;
    QLabel * m_camAltLabel;
    QLineEdit * m_camLatLine;
    QLineEdit * m_camLonLine;
    QLineEdit * m_camAltLine;
    QPushButton * m_camButton;

    QRadioButton * m_camRotate;
    QRadioButton * m_camPan;
    QRadioButton * m_camZoom;
};

#endif // MAPVIEWER_H
