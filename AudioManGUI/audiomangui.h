/**
 *
 * Copyright (C) 2011, BMW AG
 *
 * AudioManGui
 *
 * \file audiomangui.h
 *
 * \date 20.05.2011
 * \author Christian Müller (christian.ei.mueller@bmw.de)
 *
 * \section License
 * GNU Lesser General Public License, version 2.1, with special exception (GENIVI clause)
 * Copyright (C) 2011, BMW AG – Christian Müller  Christian.ei.mueller@bmw.de
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License, version 2.1, as published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License, version 2.1, for more details.
 * You should have received a copy of the GNU Lesser General Public License, version 2.1, along with this program; if not, see <http://www.gnu.org/licenses/lgpl-2.1.html>.
 * Note that the copyright holders assume that the GNU Lesser General Public License, version 2.1, may also be applicable to programs even in cases in which the program is not a library in the technical sense.
 * Linking AudioManager statically or dynamically with other modules is making a combined work based on AudioManager. You may license such other modules under the GNU Lesser General Public License, version 2.1. If you do not want to license your linked modules under the GNU Lesser General Public License, version 2.1, you may use the program under the following exception.
 * As a special exception, the copyright holders of AudioManager give you permission to combine AudioManager with software programs or libraries that are released under any license unless such a combination is not permitted by the license of such a software program or library. You may copy and distribute such a system following the terms of the GNU Lesser General Public License, version 2.1, including this special exception, for AudioManager and the licenses of the other code concerned.
 * Note that people who make modified versions of AudioManager are not obligated to grant this special exception for their modified versions; it is their choice whether to do so. The GNU Lesser General Public License, version 2.1, gives permission to release a modified version without this exception; this exception also makes it possible to release a modified version which carries forward this exception.
 */

#ifndef AUDIOMANGUI_H
#define AUDIOMANGUI_H

#include <QCoreApplication>
#include <QtGui/QWidget>
#include "qstandarditemmodel.h"
#include "ui_audiomangui.h"
#include "ui_popup_navi.h"
#include "ui_popup_ta.h"
#include "qtreewidget.h"
#include "qtreeview.h"
#include "DBusSend.h"
#include "DBusTypes.h"
#include "audiomangui.h"
#include "qlist.h"
#include "qtableview.h"
#include "qstring.h"
#include "qstringlist.h"
#include "qpainter.h"
#include "qmessagebox.h"
#include "qdial.h"

class SourceItem;
class SinkItem;
class naviPopup;
class taPopup;

struct Connection_t {
	SinkItem* Sink;
	SourceItem* Source;
};

/**\class AudioManGUI
 * \brief Main Gui Class
 * This class is used to display source sink connections and to test the AudioManager
 * \brief constructor
 * \fn AudioManGUI::insertSource(int ID, QString name);
 * \brief inserts a source
 * \fn AudioManGUI::insertSink(int ID, QString name);
 * \brief inserts a sink
 * \fn AudioManGUI::connect(int SourceID, int SinkID);
 * \brief connects source and sink
 * \fn AudioManGUI::disconnect(int SourceID, int SinkID);
 * \brief disconnects source and sink
 * \fn SinkItem* AudioManGUI::returnSelectedSink(void);
 * \brief returns the selected sink
 * \fn SourceItem* AudioManGUI::returnSelectedSource(void);
 * \brief returns the selected source
 * \fn SourceItem* AudioManGUI::returnSourceItemfromID(int ID);
 * \brief returns a source item from an ID
 * \param ID the id
 * \fn void AudioManGUI::updateButtons(void);
 * \brief is called to update the buttons
 */
class AudioManGUI: public QWidget {
Q_OBJECT

public:
	AudioManGUI(QWidget *parent = 0);
	~AudioManGUI();

	void insertSource(int ID, QString name);
	void insertSink(int ID, QString name);
	void connect(int SourceID, int SinkID);
	void disconnect(int SourceID, int SinkID);

public slots:
	void slot_selectionChanged(QTreeWidgetItem *current, int column);
	void slot_doubleClickedSource(QTreeWidgetItem *current, int column);
	void slot_connectPressed(void);
	void slot_disconnectPressed(void);
	void slot_connectionChanged(void);
	void slot_numberOfSinksChanged(void);
	void slot_numberOfSourcesChanged(void);
	void slot_volumeChanged(int);

protected:
	void paintEvent(QPaintEvent *);

private:
	SinkItem* returnSelectedSink(void);
	SourceItem* returnSelectedSource(void);
	SourceItem* returnSourceItemfromID(int ID);
	void updateButtons(void);

	Ui::AudioManGUIClass ui;
	taPopup* m_taPop;
	naviPopup* m_naviPop;
	QList<SourceItem*> m_sourceList;
	QList<SinkItem*> m_sinkList;
	QList<Connection_t> m_connectionList;
	DBusSend* sender;
	bool m_taActive;
	bool m_naviActive;

};

/**This is the navigation popup widget
 *
 */
class naviPopup: public QWidget {
Q_OBJECT
public:
	naviPopup(QWidget *parent = 0);
	virtual ~naviPopup();
private:
	Ui::naviPopup navi_pop;
};

/**The traffic announcement widget
 *
 */
class taPopup: public QWidget {
Q_OBJECT
public:
	taPopup(QWidget *parent = 0);
	virtual ~taPopup();
private:
	Ui::taPopup ta_pop;
};

/** \class SinkItem
 * \brief This class represents a sink item
 * \fn SinkItem::SinkItem(QTreeWidget* parent = 0, int ID = 0, QString Name = "");
 * \brief constructor
 * \param ID the Id of the sink
 * \param parent pointer to the parent
 * \param Name the name of the sink
 * \fn SinkItem::getSinkID(void);
 * \brief returns the Sink ID
 */
class SinkItem: SinkType, public QTreeWidgetItem {
public:
	SinkItem(QTreeWidget* parent = 0, int ID = 0, QString Name = "");
	virtual ~SinkItem();
	int getSinkID(void);
};

/**
 * \class SourceItem
 * \brief This class represents a source item
 * \fn SourceItem::SourceItem(QTreeWidget* parent = 0, int ID = 0, QString Name = "");
 * \brief constructor
 * \param ID the Id of the source
 * \param parent pointer to parent
 * \param Name the name of the source
 * \fn SourceItem::getSourceID(void);
 * \brief returns the Source ID
 * \fn SourceItem::getName(void);
 * \brief returns the name of the source item
 * \return returns the name of the source item
 */
class SourceItem: SourceType, public QTreeWidgetItem {
public:
	SourceItem(QTreeWidget* parent = 0, int ID = 0, QString Name = "");
	virtual ~SourceItem();
	int getSourceID(void);
	QString getName(void);
};

#endif // AUDIOMANGUI_H
