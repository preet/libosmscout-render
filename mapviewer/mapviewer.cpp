#include "mapviewer.h"


MapViewer::MapViewer(QWidget *parent) :
    QWidget(parent)
{
    // setup layout
    QHBoxLayout *mainLayout = new QHBoxLayout;

    // create viewport
    m_viewport = new Viewport;
    m_viewport->setFixedSize(800,480);
    mainLayout->addWidget(m_viewport);

    // vertical line
    QFrame * divViewportAndPanel = new QFrame;
    divViewportAndPanel->setFrameShape(QFrame::VLine);
    divViewportAndPanel->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(divViewportAndPanel);

    // setup side panel
    QVBoxLayout *sideLayout = new QVBoxLayout;

        // map directory line edit
        m_mapLabel = new QLabel("Map Directory:");
        m_mapLine = new QLineEdit;
        m_mapButton = new QPushButton("Browse");
        QHBoxLayout *mapLocateBox = new QHBoxLayout;
        mapLocateBox->addWidget(m_mapLine);
        mapLocateBox->addWidget(m_mapButton);
        sideLayout->addWidget(m_mapLabel);
        sideLayout->addLayout(mapLocateBox);

        // style file line edit
        m_styleLabel = new QLabel("Style File:");
        m_styleLine = new QLineEdit;
        m_styleButton = new QPushButton("Browse");
        QHBoxLayout *styleLocateBox = new QHBoxLayout;
        styleLocateBox->addWidget(m_styleLine);
        styleLocateBox->addWidget(m_styleButton);
        sideLayout->addWidget(m_styleLabel);
        sideLayout->addLayout(styleLocateBox);

        // horizontal line
        QFrame * divLoadButtonTop = new QFrame;
        divLoadButtonTop->setFrameShape(QFrame::HLine);
        divLoadButtonTop->setFrameShadow(QFrame::Sunken);
        sideLayout->addWidget(divLoadButtonTop);

        // camera settings
        m_camLatLabel = new QLabel("Latitude:");
        m_camLonLabel = new QLabel("Longitude:");
        m_camAltLabel = new QLabel("Altitude:");
        m_camLatLine = new QLineEdit;
        m_camLonLine = new QLineEdit;
        m_camAltLine = new QLineEdit;
        sideLayout->addWidget(m_camLatLabel);
        sideLayout->addWidget(m_camLatLine);
        sideLayout->addWidget(m_camLonLabel);
        sideLayout->addWidget(m_camLonLine);
        sideLayout->addWidget(m_camAltLabel);
        sideLayout->addWidget(m_camAltLine);

        QDoubleValidator * latValidator = new QDoubleValidator;
        latValidator->setRange(-90.0,90.0,7);
        m_camLatLine->setValidator(latValidator);

        QDoubleValidator * lonValidator = new QDoubleValidator;
        lonValidator->setRange(-180.0,180.0,7);
        m_camLonLine->setValidator(lonValidator);

        QDoubleValidator * altValidator = new QDoubleValidator;
        altValidator->setRange(10.0,10000.0,7);
        m_camAltLine->setValidator(altValidator);

        // horizontal line
        QFrame * divLoadButtonBtm = new QFrame;
        divLoadButtonBtm->setFrameShape(QFrame::HLine);
        divLoadButtonBtm->setFrameShadow(QFrame::Sunken);
        sideLayout->addWidget(divLoadButtonBtm);

        QHBoxLayout * buttonLayout = new QHBoxLayout;
        m_loadButton = new QPushButton("Load Map");
        m_camButton = new QPushButton("Set Camera Position");
        buttonLayout->addWidget(m_loadButton);
        buttonLayout->addWidget(m_camButton);
        sideLayout->addLayout(buttonLayout);

        // horizontal line
        QFrame * divButtonLayoutBtm = new QFrame;
        divButtonLayoutBtm->setFrameShape(QFrame::HLine);
        divButtonLayoutBtm->setFrameShadow(QFrame::Sunken);
        sideLayout->addWidget(divButtonLayoutBtm);

        m_camRotate = new QRadioButton("Rotate");
        m_camRotate->setChecked(true);
        m_camPan = new QRadioButton("Pan");
        m_camZoom = new QRadioButton("Zoom");

//        sideLayout->addWidget(m_camRotate);
//        sideLayout->addWidget(m_camPan);
//        sideLayout->addWidget(m_camZoom);

        QLabel * usageNotes = new QLabel;
        usageNotes->setWordWrap(true);
        usageNotes->setAlignment(Qt::AlignTop);
        usageNotes->setMinimumHeight(140);
        usageNotes->setText("Select a directory containing valid "
                            "map data and choose a style file. Then "
                            "click on the Load Map button to load "
                            "the map into the viewport. Use the mouse "
                            "to rotate, zoom and pan the view. The "
                            "camera's position can be manually "
                            "specified by entering the desired "
                            "latitude, longitude and altitude in "
                            "the boxes above, and then pressing "
                            "the Set Camera Position button.");
        sideLayout->addWidget(usageNotes);

    mainLayout->addLayout(sideLayout);

    // debug
    m_mapLine->setText("/home/preet/Documents/maps/toronto_render");
    m_styleLine->setText("/home/preet/Dev/projects/libosmscout-render"
                         "/libosmscout-render/styles/demo.json");

    // set size
    this->setFixedSize(800+300,480+24);

    // set layout
    this->setLayout(mainLayout);

    // set title
    this->setWindowTitle("libosmscout-render mapviewer");

    // setup connections
    connect(this, SIGNAL(loadMap(QString,QString)),
            m_viewport,SLOT(onLoadMap(QString,QString)));

    connect(this, SIGNAL(setCameraLLA(double,double,double)),
            m_viewport,SLOT(onSetCameraLLA(double,double,double)));

    connect(m_loadButton,SIGNAL(clicked()),
            this,SLOT(onLoadButtonClicked()));

    connect(m_camButton,SIGNAL(clicked()),
            this,SLOT(onCamButtonClicked()));

    connect(m_camRotate, SIGNAL(clicked()),
            this,SLOT(onCamModeRotate()));

    connect(m_camPan, SIGNAL(clicked()),
            this,SLOT(onCamModePan()));

    connect(m_camZoom, SIGNAL(clicked()),
            this,SLOT(onCamModeZoom()));

    connect(this,SIGNAL(setCamMode(int)),
            m_viewport,SLOT(onSetCameraMouseMode(int)));
}

MapViewer::~MapViewer()
{

}

void MapViewer::onLoadButtonClicked()
{
    emit loadMap(m_mapLine->text(),m_styleLine->text());
}

void MapViewer::onCamButtonClicked()
{
    double camLat = m_camLatLine->text().toDouble();
    double camLon = m_camLonLine->text().toDouble();
    double camAlt = m_camAltLine->text().toDouble();
    emit setCameraLLA(camLat,camLon,camAlt);
}

void MapViewer::onCamModeRotate()
{
    emit setCamMode(0);
}

void MapViewer::onCamModePan()
{
    emit setCamMode(1);
}

void MapViewer::onCamModeZoom()
{
    emit setCamMode(2);
}

