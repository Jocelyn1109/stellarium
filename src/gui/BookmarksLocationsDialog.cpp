/*
 * Stellarium
 * 
 * Copyright (C) 2019 Jocelyn GIROD
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA  02110-1335, USA.
*/

#include <QStringBuilder>
#include <QDir>

#include "BookmarksLocationsDialog.hpp"
#include "ui_bookmarksLocationsDialog.h"

#include "StelCore.hpp"
#include "StelUtils.hpp"
#include "StelObjectMgr.hpp"
#include "StelModuleMgr.hpp"
#include "StelFileMgr.hpp"
#include "LabelMgr.hpp"
#include "StelTranslator.hpp"
#include "StelLocation.hpp"
#include "StelJsonParser.hpp"

//Constructor
BookmarksLocationsDialog::BookmarksLocationsDialog(QObject* parent): StelDialog("BookmarksLocations", parent)
{
    ui = new Ui_bookmarksLocationsDialogForm;
    core = StelApp::getInstance().getCore();
    objectMgr = GETSTELMODULE(StelObjectMgr);
	labelMgr = GETSTELMODULE(LabelMgr);
    bookmarksLocationsListModel = new QStandardItemModel(0, ColumnCount);
    bookmarksLocationsJsonPath = StelFileMgr::findFile("data", (StelFileMgr::Flags)(StelFileMgr::Directory|StelFileMgr::Writable)) + "/" + FILE_NAME;
}


//Destructor
BookmarksLocationsDialog::~BookmarksLocationsDialog()
{
    delete ui;
}


void BookmarksLocationsDialog::styleChanged()
{
	// Nothing for now
}


/**
 * Create and initialize the dialog content
 */
void BookmarksLocationsDialog::createDialogContent()
{
    ui->setupUi(dialog);
    
    //Signals and slots
	connect(&StelApp::getInstance(), SIGNAL(languageChanged()), this, SLOT(retranslate()));
	connect(ui->closeStelWindow, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui->TitleBar, SIGNAL(movedTo(QPoint)), this, SLOT(handleMovedTo(QPoint)));
    
    //Signals and slots - Buttons
    connect(ui->addBookmarkCurrentLocationButton, SIGNAL(clicked()), this, SLOT(addBookmarkCurrentLocationButtonPressed()));
    connect(ui->addBookmarkLocationFromListButton, SIGNAL(clicked()), this, SLOT(addBookmarkLocationFromListButtonPressed()));
    connect(ui->addBookmarkLocationFromGpsCoordButton, SIGNAL(clicked()), this, SLOT(addBookmarkLocationFromGpsCoordButtonPressed()));
    connect(ui->goToLocationButton, SIGNAL(clicked()), this, SLOT(goToLocationButtonPressed()));
    connect(ui->importBookmarksLocationsButton, SIGNAL(clicked()), this, SLOT(importBookmarksLocationsButtonPressed()));
    connect(ui->exportBookmarksLocationsButton, SIGNAL(clicked()), this, SLOT(exportBookmarksLocationsButtonPressed()));
    connect(ui->removeBookmarkLocationButton, SIGNAL(clicked()), this, SLOT(removeBookmarkLocationButtonPressed()));
    connect(ui->clearBookmarksLocationsButton, SIGNAL(clicked()), this, SLOT(clearBookmarksLocationsButtonPressed()));
    
    //Initializing the list of bookmarks
    bookmarksLocationsListModel->setColumnCount(BookmarksLocationsColumns::ColumnCount);
    setBookmarksLocationsHeaderNames();
    
    ui->bookmarksLocationsTreeView->setModel(bookmarksLocationsListModel);
	ui->bookmarksLocationsTreeView->header()->setSectionsMovable(false);
	ui->bookmarksLocationsTreeView->header()->setSectionResizeMode(ColumnName, QHeaderView::ResizeToContents);
	ui->bookmarksLocationsTreeView->header()->setStretchLastSection(true);
	ui->bookmarksLocationsTreeView->hideColumn(BookmarksLocationsColumns::ColumnUUID);
    
    QString style = "QCheckBox { color: rgb(238, 238, 238); }";
	ui->dateTimeCheckBox->setStyleSheet(style);
    
    //Enable the sort for columns
    ui->bookmarksLocationsTreeView->setSortingEnabled(true);
}


// Define the header name for bookmarks locations table
void BookmarksLocationsDialog::setBookmarksLocationsHeaderNames()
{
    QStringList headerStrings;
	headerStrings << "UUID"; // Hide the column
	headerStrings << q_("Location");
	headerStrings << q_("Date and time");	
	headerStrings << q_("Latitude");	
	headerStrings << q_("Longitude");
    
    bookmarksLocationsListModel->setHorizontalHeaderLabels(headerStrings);
}

//Add model raw in the model list (bookmarksLocationsListModel)
void BookmarksLocationsDialog::addModelRow(int number, QString uuid, QString name, QString date, QString latitude, QString longitude)
{
    QStandardItem* item = Q_NULLPTR;

	item = new QStandardItem(uuid);
	item->setEditable(false);
	bookmarksLocationsListModel->setItem(number, BookmarksLocationsColumns::ColumnUUID, item);

	item = new QStandardItem(name);
	item->setEditable(false);
	bookmarksLocationsListModel->setItem(number, BookmarksLocationsColumns::ColumnName, item);

	item= new QStandardItem(date);
	item->setEditable(false);
	bookmarksLocationsListModel->setItem(number, BookmarksLocationsColumns::ColumnDate, item);

	item = new QStandardItem(latitude);
	item->setEditable(false);
	bookmarksLocationsListModel->setItem(number, BookmarksLocationsColumns::ColumnLatitude, item);

	item = new QStandardItem(longitude);
	item->setEditable(false);
	bookmarksLocationsListModel->setItem(number, BookmarksLocationsColumns::ColumnLongitude, item);

	for(int i = 0; i < ColumnCount; ++i)
	{
		ui->bookmarksLocationsTreeView->resizeColumnToContents(i);
	}
}


void BookmarksLocationsDialog::retranslate()
{
	if (dialog)
	{
        ui->retranslateUi(dialog);
        setBookmarksLocationsHeaderNames();
	}
}


// Slot for add bookmarls current location button
void BookmarksLocationsDialog::addBookmarkCurrentLocationButtonPressed()
{
    const StelLocation currentLocation = core->getCurrentLocation();
    QString locationToDisplay = currentLocation.name % ", " % currentLocation.country;
    if(currentLocation.state != "")
    {
        locationToDisplay = locationToDisplay % "(" % currentLocation.state % ")";
    }
    
    QString longitude = QString::number(currentLocation.longitude);
    QString latitude = QString::number(currentLocation.latitude);
    
    bool dateTimeFlag = ui->dateTimeCheckBox->isChecked();
    QString JDs = "";
    double JD = -1.;
    if(dateTimeFlag)
    {
        JD = core->getJD();
        JDs = StelUtils::julianDayToISO8601String(JD + core->getUTCOffset(JD)/24.).replace("T", " ");
    }
    
    int lastRow = bookmarksLocationsListModel->rowCount();
    QString uuid = QUuid::createUuid().toString();
    addModelRow(lastRow, uuid, locationToDisplay, JDs, latitude, longitude);
    
    bookmarkLocation bml;
    bml.name = locationToDisplay;
    bml.latitude = latitude;
    bml.longitude = longitude;
    bml.jd = JDs;
    bookmarksCollection.insert(uuid, bml);
    
    saveBookmarks();
}

//! Slot for add bookmark location from list
void BookmarksLocationsDialog::addBookmarkLocationFromListButtonPressed()
{
    //TODO: à implémenter
}

//! Slot for add bookmarks location for GPS coordinates button
void BookmarksLocationsDialog::addBookmarkLocationFromGpsCoordButtonPressed()
{
    //TODO: à implémenter
}

//! Slot for remove bookmark location button
void BookmarksLocationsDialog::removeBookmarkLocationButtonPressed()
{
    //TODO: à implémenter
}

//! Slot for clear bookmarks locations button
void BookmarksLocationsDialog::clearBookmarksLocationsButtonPressed()
{
    //TODO: à implémenter
}

//! Slot for import bookmarks location button
void BookmarksLocationsDialog::importBookmarksLocationsButtonPressed()
{
    //TODO: à implémenter
}

//! Slot for export bookmarks location button
void BookmarksLocationsDialog::exportBookmarksLocationsButtonPressed()
{
    //TODO: à implémenter//TODO: à implémenter
}

//! Slot for go to location button
void BookmarksLocationsDialog::goToLocationButtonPressed()
{
    //TODO: à implémenter
}

//! Set the current location into the bookmarks file
void BookmarksLocationsDialog::selectCurrentBookmarkLocation(const QModelIndex& modelIdx)
{
    //TODO: à implémenter
}

//! Save the bookmarks in json file
void BookmarksLocationsDialog::saveBookmarks() const
{
    if(bookmarksLocationsJsonPath.isEmpty())
	{
		qWarning() << "[Bookmarks Locations] Error saving bookmarks";
		return;
        
	} else if(bookmarksCollection.isEmpty()){
        qWarning() << "[Bookmarks Locations] Error there are no bookmarks to save";
        return;
        
    }
	
	QFile jsonFile(bookmarksLocationsJsonPath);
	if(!jsonFile.open(QFile::WriteOnly|QFile::Text))
	{
		qWarning() << "[Bookmarks Locations] bookmarks can not be saved. A file can not be open for writing:"
			   << QDir::toNativeSeparators(bookmarksLocationsJsonPath);
		return;
	}
	
	QVariantMap bookmarksLocationDataList;
	QHashIterator<QString, bookmarkLocation> i(bookmarksCollection);
    while (i.hasNext())
	{
	    i.next();

	    bookmarkLocation sp = i.value();
	    QVariantMap bm;
	    bm.insert("name", sp.name);
        if(!sp.name.isEmpty())
        {
            bm.insert("name" , sp.name);
        }
        if(!sp.latitude.isEmpty())
        {
            bm.insert("latitude" , sp.latitude);
        }
        if(!sp.longitude.isEmpty())
        {
            bm.insert("longitude" , sp.longitude);
        }
        if(!sp.jd.isEmpty())
        {
            bm.insert("jd" , sp.jd);
        }

	    bookmarksLocationDataList.insert(i.key(), bm);
	}
	
	QVariantMap bmLocList;
	bmLocList.insert("bookmarks locations", bookmarksLocationDataList);

	//Convert the tree to JSON
	StelJsonParser::write(bmLocList, &jsonFile);
	jsonFile.flush();
	jsonFile.close();
}






    


