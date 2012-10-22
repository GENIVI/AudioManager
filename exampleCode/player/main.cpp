/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** GNU General Public License Usage
** This file is licensed under GPL v2.
**
**
** $QT_END_LICENSE$
**
***************************************************************************/

#include <QtGui>
#include "mediaplayer.h"
#include "dbushandler.h"
#include "audiomanagerinteractor.h"

const qreal DefaultVolume = -1.0;

int main (int argc, char *argv[])
{
    Q_INIT_RESOURCE(mediaplayer);
    QApplication app(argc, argv);

    QStringList args = app.arguments();
    app.setApplicationName(args.at(1));
    std::string appName=args.at(1).toStdString();
    std::string targetName=args.at(2).toStdString();

    app.setOrganizationName("Genivi");
    app.setQuitOnLastWindowClosed(true);

    QString fileName;
    qreal volume = DefaultVolume;
    bool smallScreen = false;
#ifdef Q_OS_SYMBIAN
    smallScreen = true;
#endif

    am_sourceID_t mySourceID=0;
    am_sinkID_t targetSinkID=0;

    DbusHandler dbusHandler;

    //first we need to find out our sourceID
    std::vector<am_SourceType_s> listSources;
    std::vector<am_SinkType_s> listSinks;
    dbusHandler.GetListMainSources(listSources);
    dbusHandler.GetListMainSinks(listSinks);

    std::vector<am_SourceType_s>::iterator sourceIter(listSources.begin());
    for (;sourceIter!=listSources.end();++sourceIter)
    {
        if (sourceIter->name.compare(appName)==0)
            mySourceID=sourceIter->sourceID;
    }

    std::vector<am_SinkType_s>::iterator sinkIter(listSinks.begin());
    for (;sinkIter!=listSinks.end();++sinkIter)
    {

        if (sinkIter->name.compare(targetName)==0)
            targetSinkID=sinkIter->sinkID;
    }

    std::cout<< "SourceID "<<mySourceID<<std::endl;

    QString appNameSourceID=args.at(1)+" sourceID="+QString::number(mySourceID);

    AudioManagerInteractor interActor(&dbusHandler,mySourceID,targetSinkID);


    MediaPlayer player(appNameSourceID);
    player.setSmallScreen(smallScreen);
    if (DefaultVolume != volume)
        player.setVolume(volume);
    if (!fileName.isNull())
        player.setFile(fileName);

    if (smallScreen)
        player.showMaximized();
    else
        player.show();

    QObject::connect((const QObject*)player.playButton, SIGNAL(clicked()),&interActor, SLOT(playPause()));
    QObject::connect((const QObject*)&dbusHandler, SIGNAL(SourceActivity(am_sourceID_t ,am_SourceState_e )),&interActor, SLOT(SourceActivity(am_sourceID_t ,am_SourceState_e )));
    QObject::connect((const QObject*)&interActor, SIGNAL(play()),&player, SLOT(play()));
    QObject::connect((const QObject*)&interActor, SIGNAL(stop()),&player, SLOT(stop()));
    QObject::connect((const QObject*)&player, SIGNAL(MediaStateChanged(Phonon::State)),&interActor, SLOT(getPlayerState(Phonon::State)));

    return app.exec();
}

