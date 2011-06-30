/**
 * Copyright (C) 2011, BMW AG
 *
 * AudioManGui
 *
 * \file audiomangui.cpp
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

#include "audiomangui.h"

AudioManGUI::AudioManGUI(QWidget *parent) :
	m_taPop(new taPopup()), m_naviPop(new naviPopup()) {

	(void)parent;
	ui.setupUi(this);
	ui.SourceView->setHeaderLabel("Sources");
	ui.SinkView->setHeaderLabel("Sinks");
	QObject::connect((const QObject*) ui.SourceView, SIGNAL (itemClicked(QTreeWidgetItem*,int)), (const QObject*) this, SLOT (slot_selectionChanged(QTreeWidgetItem *, int)));
	QObject::connect((const QObject*) ui.SinkView, SIGNAL (itemClicked(QTreeWidgetItem*,int)), (const QObject*) this, SLOT (slot_selectionChanged(QTreeWidgetItem *, int)));
	QObject::connect((const QObject*) ui.pushButton, SIGNAL(pressed()), (const QObject*) this, SLOT(slot_connectPressed()));
	QObject::connect((const QObject*) ui.pushButton_2, SIGNAL(pressed()), (const QObject*) this, SLOT(slot_disconnectPressed()));
	QObject::connect((const QObject*) ui.SinkView, SIGNAL (itemDoubleClicked(QTreeWidgetItem*,int)), (const QObject*) this, SLOT (slot_doubleClickedSource(QTreeWidgetItem *, int)));

	//setup DBus Connection
	REGISTER_METATYPES
	sender = new DBusSend("org.Genivi.ControllerInterface", "/Control", QDBusConnection::sessionBus(), 0);

	QObject::connect((const QObject*) sender, SIGNAL(signal_connectionChanged()), (const QObject*) this, SLOT(slot_connectionChanged()));
	QObject::connect((const QObject*) sender, SIGNAL(signal_numberOfSinksChanged()), (const QObject*) this, SLOT(slot_numberOfSinksChanged()));
	QObject::connect((const QObject*) sender, SIGNAL(signal_numberOfSourcesChanged()), (const QObject*) this, SLOT(slot_numberOfSourcesChanged()));

	m_taActive = false;
	m_naviActive = false;

	//initial fill
	QList<SourceType> sources = sender->getListSources();
	foreach (SourceType s,sources) {
		this->insertSource(s.ID, s.name);
	}

	QList<SinkType> sinks = sender->getListSinks();
	foreach (SinkType s, sinks) {
		this->insertSink(s.ID, s.name);
	}

	QList<ConnectionType> connections = sender->getListConnections();
	foreach (ConnectionType c, connections) {
		this->connect(c.Source_ID, c.Sink_ID);
	}
}

AudioManGUI::~AudioManGUI() {
	delete m_taPop;
	delete m_naviPop;

}

void AudioManGUI::insertSource(int ID, QString name) {
	SourceItem* Source = new SourceItem(ui.SourceView, ID, name);
	m_sourceList.append(Source);
}

void AudioManGUI::insertSink(int ID, QString name) {
	SinkItem* Sink = new SinkItem(ui.SinkView, ID, name);
	m_sinkList.append(Sink);
}


void AudioManGUI::connect(int SourceID, int SinkID) {
	Connection_t connect;
	foreach (SourceItem* Source, m_sourceList)
		{
			if (SourceID == Source->getSourceID()) {
				connect.Source = Source;
			}
		}

	foreach (SinkItem* Sink,m_sinkList)
		{
			if (SinkID == Sink->getSinkID()) {
				connect.Sink = Sink;
			}
		}
	m_connectionList.append(connect);
	QWidget::update();
}

SinkItem* AudioManGUI::returnSelectedSink(void) {
	SinkItem* SinkI = NULL;

	QList<QTreeWidgetItem*> SinkL = ui.SinkView->selectedItems();
	if (SinkL.size() > 0) {
		SinkI = static_cast<SinkItem*> (SinkL.first());
	}
	return SinkI;
}

SourceItem* AudioManGUI::returnSelectedSource(void) {
	SourceItem* SourceI = NULL;

	QList<QTreeWidgetItem*> SourceL = ui.SourceView->selectedItems();
	if (SourceL.size() > 0) {
		SourceI = static_cast<SourceItem*> (SourceL.first());
	}
	return SourceI;
}

SourceItem* AudioManGUI::returnSourceItemfromID(int ID) {
	foreach (SourceItem* i, m_sourceList) {
		if (i->getSourceID() == ID) {
			return i;
		}
	}
	return NULL;

}

void AudioManGUI::paintEvent(QPaintEvent * event) {
	(void)event;
	QPainter painter(this);
	QPen pen(Qt::black, 2, Qt::SolidLine);
	painter.setPen(pen);
	QRect rect;
	int x1 = ui.SourceView->geometry().x() + ui.SourceView->geometry().width() + 5;
	int x2 = ui.SinkView->geometry().x() - 5;
	rect = ui.SourceView->visualItemRect(ui.SourceView->topLevelItem(0));
	int y1 = rect.height();
	int ylevel = ui.SinkView->geometry().topLeft().y() + y1 * 1.5;

	foreach (Connection_t con, m_connectionList) {
		rect = ui.SourceView->visualItemRect(con.Source);
		int y1 = (rect.top() + rect.height() / 2) + ylevel;

		rect = ui.SinkView->visualItemRect(con.Sink);
		int y2 = (rect.top() + rect.height() / 2) + ylevel;

		painter.drawLine(QPoint(x1, y1), QPoint(x2, y2));
	}
}

void AudioManGUI::slot_connectPressed() {
	SourceItem* SourceI = returnSelectedSource();
	SinkItem* SinkI = returnSelectedSink();

	if (SourceI && SinkI) {
		connect(SourceI->getSourceID(), SinkI->getSinkID());
		sender->connect(SourceI->getSourceID(), SinkI->getSinkID());
	}
	updateButtons();
}

void AudioManGUI::slot_disconnectPressed() {
	SourceItem* SourceI = returnSelectedSource();
	SinkItem* SinkI = returnSelectedSink();

	if (SourceI && SinkI) {
		sender->disconnect(SourceI->getSourceID(), SinkI->getSinkID());
	}
	updateButtons();
}

void AudioManGUI::updateButtons() {
	int rem = 0;
	SourceItem* SourceI = returnSelectedSource();
	SinkItem* SinkI = returnSelectedSink();

	if (SourceI && SinkI) {
		int sourceID = SourceI->getSourceID();
		int sinkID = SinkI->getSinkID();
		foreach (Connection_t con,m_connectionList)
			{
				if (sinkID == con.Sink->getSinkID() && sourceID == con.Source->getSourceID()) {
					rem = 1;
				}
			}
	}

	if (rem == 1) {
		ui.pushButton_2->setEnabled(true);
		ui.pushButton->setEnabled(false);
	} else {
		ui.pushButton->setEnabled(true);
		ui.pushButton_2->setEnabled(false);
	}

	QWidget::update();
}

void AudioManGUI::slot_selectionChanged(QTreeWidgetItem *current, int column) {
	(void) current;
	(void) column;
	updateButtons();
}

void AudioManGUI::slot_doubleClickedSource(QTreeWidgetItem *current, int column) {
	(void) current;
	(void) column;
	QFrame *popup1;
	popup1 = new QFrame(this, Qt::Popup);
	popup1->resize(30, 100);
	QRect rect = ui.SinkView->visualItemRect(current);
	int width = ui.horizontalSpacer_4->geometry().width() + ui.horizontalSpacer_3->geometry().width() + ui.horizontalSpacer_5->geometry().width() + ui.pushButton_2->geometry().width();
	rect.setBottomRight(QPoint(rect.bottomRight().x() + width, rect.bottomRight().y()));
	popup1->move(mapToGlobal(rect.bottomRight()));

	QSlider* slider;
	slider = new QSlider(popup1);
	slider->resize(30, 100);
	slider->setMaximum(0xFFFF);
	slider->show();
	popup1->show();
	QObject::connect((const QObject*) slider, SIGNAL(valueChanged(int)), (const QObject*) this, SLOT(slot_volumeChanged(int)));
}

void AudioManGUI::slot_volumeChanged(int volume) {
	SinkItem* SinkI = returnSelectedSink();
	sender->setVolume(SinkI->getSinkID(), volume);

}

void AudioManGUI::slot_connectionChanged() {
	QList<ConnectionType> connections = sender->getListConnections();
	m_connectionList.clear();
	m_taActive = false;
	m_naviActive = false;
	foreach (ConnectionType c, connections) {
		this->connect(c.Source_ID, c.Sink_ID);
		if (returnSourceItemfromID(c.Source_ID)->getName().compare("ta") == 0) {
			m_taActive = true;
		} else if (returnSourceItemfromID(c.Source_ID)->getName().compare("navigation") == 0) {
			m_naviActive = true;
		}
	}

	//state changed?
	if (m_taActive) {
		m_taPop->show();
		m_taPop->move(this->geometry().center());
	} else {
		m_taPop->hide();
	}

	if (m_naviActive) {
		m_naviPop->show();
		m_taPop->move(mapToGlobal(ui.SourceView->pos()));
	} else {
		m_naviPop->hide();
	}

	QWidget::update();
}

void AudioManGUI::slot_numberOfSinksChanged() {
	QList<SinkType> sinks = sender->getListSinks();
	foreach (SinkType s, sinks) {
		bool exist = false;
		foreach (SinkItem* item, m_sinkList)
			{
				if (item->getSinkID() == s.ID) {
					exist = true;
				}
			}
		if (!exist) {
			this->insertSink(s.ID, s.name);
			QWidget::update();
		}
	}
}

void AudioManGUI::slot_numberOfSourcesChanged() {
	QList<SourceType> sources = sender->getListSources();
	foreach (SourceType s,sources) {
		bool exist = false;
		foreach (SourceItem* item, m_sourceList) {
			if (item->getSourceID() == s.ID) {
				exist = true;
			}
		}
		if (!exist) {
			this->insertSource(s.ID, s.name);
			QWidget::update();
		}
	}
}

naviPopup::naviPopup(QWidget* parent) {
	(void)parent;
	navi_pop.setupUi(this);
}

naviPopup::~naviPopup() {

}

taPopup::taPopup(QWidget* parent) {
	(void) parent;
	ta_pop.setupUi(this);
}

taPopup::~taPopup() {

}

SinkItem::SinkItem(QTreeWidget* parent, int givenID, QString givenName) {
	ID = givenID;
	name = givenName;
	QTreeWidgetItem::setText(0, givenName);
	parent->insertTopLevelItem(0, this);
}

int SinkItem::getSinkID() {
	return ID;
}

SinkItem::~SinkItem() {
}

SourceItem::SourceItem(QTreeWidget* parent, int givenID, QString givenName) {
	ID = givenID;
	name = givenName;
	QTreeWidgetItem::setText(0, givenName);
	parent->insertTopLevelItem(0, this);
}

QString SourceItem::getName() {
	return name;
}

int SourceItem::getSourceID() {
	return ID;
}

SourceItem::~SourceItem() {

}

