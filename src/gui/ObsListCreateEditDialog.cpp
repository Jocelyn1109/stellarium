/*
 * * Stellarium
 * Copyright (C) 2020 Jocelyn GIROD
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <StelTranslator.hpp>
#include <QUuid>
#include <QDir>
#include <QDateTime>

#include "StelCore.hpp"
#include "StelFileMgr.hpp"
#include "StelObjectMgr.hpp"
#include "StelModuleMgr.hpp"
#include "NebulaMgr.hpp"
#include "StelMovementMgr.hpp"
#include "StelUtils.hpp"
#include "StelJsonParser.hpp"

#include "ObsListCreateEditDialog.hpp"
#include "ui_obsListCreateEditDialog.h"

using namespace std;

ObsListCreateEditDialog * ObsListCreateEditDialog::m_instance = nullptr;

ObsListCreateEditDialog::ObsListCreateEditDialog ( string listName )
{
    listName_ = listName;

    if ( listName_.size() == 0 ) {
        // case of creation mode
        isCreationMode = true;
    } else {
        // case of edit mode
        isCreationMode = false;
        loadObservingList();
    }

    ui = new Ui_obsListCreateEditDialogForm();
    core = StelApp::getInstance().getCore();
    objectMgr = GETSTELMODULE ( StelObjectMgr );
    obsListListModel = new QStandardItemModel ( 0,ColumnCount );
    observingListJsonPath = StelFileMgr::findFile ( "data", ( StelFileMgr::Flags ) ( StelFileMgr::Directory|StelFileMgr::Writable ) ) + "/" + QString ( jsonFileName );
}

ObsListCreateEditDialog::~ObsListCreateEditDialog()
{
    delete ui;
    delete obsListListModel;
}

/**
 * Get instance of class
*/
ObsListCreateEditDialog * ObsListCreateEditDialog::Instance ( string listName )
{
    if ( m_instance == nullptr ) {
        m_instance = new ObsListCreateEditDialog ( listName );
    }

    return m_instance;
}


/*
 * Initialize the dialog widgets and connect the signals/slots.
*/
void ObsListCreateEditDialog::createDialogContent()
{
    ui->setupUi ( dialog );

    //Signals and slots
    connect ( &StelApp::getInstance(), SIGNAL ( languageChanged() ), this, SLOT ( retranslate() ) );
    connect ( ui->closeStelWindow, SIGNAL ( clicked() ), this, SLOT ( close() ) );

    connect ( ui->obsListAddObjectButton, SIGNAL ( clicked() ), this, SLOT ( obsListAddObjectButtonPressed() ) );
    connect ( ui->obsListExitButton, SIGNAL ( clicked() ), this, SLOT ( obsListExitButtonPressed() ) );
    connect ( ui->obsListSaveButton, SIGNAL ( clicked() ), this, SLOT ( obsListSaveButtonPressed() ) );
    connect ( ui->obsListRemoveObjectButton, SIGNAL ( clicked() ), this, SLOT ( obsListRemoveObjectButtonPressed() ) );
    connect ( ui->obsListImportListButton, SIGNAL ( clicked() ), this, SLOT ( obsListImportListButtonPresssed() ) );
    connect ( ui->obsListExportListButton, SIGNAL ( clicked() ), this, SLOT ( obsListExportListButtonPressed() ) );

    //Initializing the list of observing list
    obsListListModel->setColumnCount ( ColumnCount );
    setObservingListHeaderNames();

    ui->obsListCreationEditionTreeView->setModel ( obsListListModel );
    ui->obsListCreationEditionTreeView->header()->setSectionsMovable ( false );
    ui->obsListCreationEditionTreeView->header()->setSectionResizeMode ( ColumnName, QHeaderView::ResizeToContents );
    ui->obsListCreationEditionTreeView->header()->setStretchLastSection ( true );
    ui->obsListCreationEditionTreeView->hideColumn ( ColumnUUID );
    ui->obsListCreationEditionTreeView->hideColumn ( ColumnNameI18n );
    ui->obsListCreationEditionTreeView->hideColumn ( ColumnJD );
    ui->obsListCreationEditionTreeView->hideColumn ( ColumnLocation );
    //Enable the sort for columns
    ui->obsListCreationEditionTreeView->setSortingEnabled ( true );
}

/*
 * Retranslate dialog
*/
void ObsListCreateEditDialog::retranslate()
{
    if ( dialog ) {
        ui->retranslateUi ( dialog );
    }
}

/*
 * Style changed
*/
void ObsListCreateEditDialog::styleChanged()
{
    // Nothing for now
}

/*
 * Set the header for the observing list table
 * (obsListTreeVView)
*/
void ObsListCreateEditDialog::setObservingListHeaderNames()
{
    const QStringList headerStrings = {
        "UUID", // Hided column
        q_ ( "Object name" ),
        q_ ( "Object Name I18N" ),
        q_ ( "Type" ),
        q_ ( "Right ascencion" ),
        q_ ( "Declination" ),
        q_ ( "Magnitude" ),
        q_ ( "Constellation" ),
        q_ ( "Date" ), // Hided column
        q_ ( "Location" ) // Hided column
    };

    obsListListModel->setHorizontalHeaderLabels ( headerStrings );
}


/*
 * Add row in the obsListListModel
*/
void ObsListCreateEditDialog::addModelRow ( int number, QString uuid, QString name, QString nameI18n, QString type, QString ra, QString dec, QString magnitude, QString constellation )
{
    QStandardItem* item = Q_NULLPTR;

    item = new QStandardItem ( uuid );
    item->setEditable ( false );
    obsListListModel->setItem ( number, ColumnUUID, item );

    item = new QStandardItem ( name );
    item->setEditable ( false );
    obsListListModel->setItem ( number, ColumnName, item );

    item = new QStandardItem ( nameI18n );
    item->setEditable ( false );
    obsListListModel->setItem ( number, ColumnNameI18n, item );

    item = new QStandardItem ( type );
    item->setEditable ( false );
    obsListListModel->setItem ( number, ColumnType, item );

    item = new QStandardItem ( ra );
    item->setEditable ( false );
    obsListListModel->setItem ( number, ColumnRa, item );

    item = new QStandardItem ( dec );
    item->setEditable ( false );
    obsListListModel->setItem ( number, ColumnDec, item );

    item = new QStandardItem ( magnitude );
    item->setEditable ( false );
    obsListListModel->setItem ( number, ColumnMagnitude, item );

    item = new QStandardItem ( constellation );
    item->setEditable ( false );
    obsListListModel->setItem ( number, ColumnConstellation, item );

    for ( int i = 0; i < ColumnCount; ++i ) {
        ui->obsListCreationEditionTreeView->resizeColumnToContents ( i );
    }
}


/*
 * Slot for button obsListAddObjectButton
*/
void ObsListCreateEditDialog::obsListAddObjectButtonPressed()
{
    const QList<StelObjectP>& selectedObject = objectMgr->getSelectedObject();
    if ( !selectedObject.isEmpty() ) {

        int lastRow = obsListListModel->rowCount();
        QString objectUuid = QUuid::createUuid().toString();

        QString objectName = selectedObject[0]->getEnglishName();
        QString objectNameI18n = selectedObject[0]->getNameI18n();
        if ( selectedObject[0]->getType() =="Nebula" ) {
            objectName = GETSTELMODULE ( NebulaMgr )->getLatestSelectedDSODesignation();
        }

        QString objectRaStr = "", objectDecStr = "";
        bool visibleFlag = false;
        double fov = -1.0;


        QString objectType = selectedObject[0]->getType();
        if ( objectName.isEmpty() || objectType =="CustomObject" ) {
            float ra, dec;
            StelUtils::rectToSphe ( &ra, &dec, selectedObject[0]->getJ2000EquatorialPos ( core ) );
            objectRaStr = StelUtils::radToHmsStr ( ra, false ).trimmed();
            objectDecStr = StelUtils::radToDmsStr ( dec, false ).trimmed();
            if ( objectName.contains ( "marker", Qt::CaseInsensitive ) ) {
                visibleFlag = true;
            }

            if ( objectName.isEmpty() ) {
                objectName = QString ( "%1, %2" ).arg ( objectRaStr, objectDecStr );
                objectNameI18n = q_ ( "Unnamed object" );
                fov = GETSTELMODULE ( StelMovementMgr )->getCurrentFov();
            }

        }




        float objectMagnitude = selectedObject[0]->getVMagnitude ( core );
        QString objectMagnitudeStr = QString::number ( objectMagnitude );

        QVariantMap objectMap = selectedObject[0]->getInfoMap ( core );
        QVariant objectConstellationVariant = objectMap["iauConstellation"];
        QString objectConstellation ( "unknown" );
        if ( objectConstellationVariant.canConvert<QString>() ) {
            objectConstellation = objectConstellationVariant.value<QString>();
        }

        QString JDs = "";
        double JD = -1.;
        JD = core->getJD();
        JDs = StelUtils::julianDayToISO8601String ( JD + core->getUTCOffset ( JD ) /24. ).replace ( "T", " " );

        QString Location = "";
        StelLocation loc = core->getCurrentLocation();
        if ( loc.name.isEmpty() ) {
            Location = QString ( "%1, %2" ).arg ( loc.latitude ).arg ( loc.longitude );
        } else {
            Location = QString ( "%1, %2" ).arg ( loc.name ).arg ( loc.country );
        }

        addModelRow ( lastRow,objectUuid,objectName, objectNameI18n, objectType, objectRaStr, objectDecStr, objectMagnitudeStr, objectConstellation );

        observingListItem item;
        item.name = objectName;
        item.nameI18n = objectNameI18n;
        if ( !objectType.isEmpty() ) {
            item.type = objectType;
        }
        if ( !objectRaStr.isEmpty() ) {
            item.ra = objectRaStr;
        }
        if ( !objectDecStr.isEmpty() ) {
            item.dec = objectDecStr;
        }
        if ( !objectMagnitudeStr.isEmpty() ) {
            item.magnitude = objectMagnitudeStr;
        }
        if ( !objectConstellation.isEmpty() ) {
            item.constellation = objectConstellation;
        }
        if ( !JDs.isEmpty() ) {
            item.jd = JDs;
        }
        if ( !Location.isEmpty() ) {
            item.location = Location;
        }
        if ( !visibleFlag ) {
            item.isVisibleMarker = visibleFlag;
        }
        if ( fov > 0.0 ) {
            item.fov = fov;
        }

        observingListItemCollection.insert ( objectUuid,item );

        saveObservedObject();

    } else {
        qWarning() << "selected object is empty !";
    }
}


/*
 * Save observed object into json file
*/
void ObsListCreateEditDialog::saveObservedObject()
{

    QString listName = ui->nameOfListLineEdit->text();
    if ( observingListJsonPath.isEmpty() || listName.isEmpty() ) {
        qWarning() << "[ObservingList] Error saving observing list";
        return;
    }

    QFile jsonFile ( observingListJsonPath );
    if ( !jsonFile.open ( QFile::WriteOnly|QFile::Text ) ) {
        qWarning() << "[ObservingList] observing list can not be saved. A file can not be open for writing:"
                   << QDir::toNativeSeparators ( observingListJsonPath );
        return;
    }

    QVariantMap observingListDataList;
    QString oblListUuid = QUuid::createUuid().toString();
    observingListDataList.insert ( "UUID",  oblListUuid);
    QString currentDateTime = QDateTime::currentDateTime().toString();
    observingListDataList.insert ( "Current date time", currentDateTime );

    QHashIterator<QString, observingListItem> i ( observingListItemCollection );
    while ( i.hasNext() ) {
        i.next();

        observingListItem item = i.value();
        QVariantMap obl;
        obl.insert ( "name", item.name );
        if ( !item.nameI18n.isEmpty() ) {
            obl.insert ( "nameI18n", item.nameI18n );
        }
        if ( !item.type.isEmpty() ) {
            obl.insert ( "type", item.type );
        }
        if ( !item.ra.isEmpty() ) {
            obl.insert ( "ra", item.ra );
        }
        if ( !item.dec.isEmpty() ) {
            obl.insert ( "dec", item.dec );
        }
        if ( !item.magnitude.isEmpty() ) {
            obl.insert ( "magnitude", item.magnitude );
        }
        if ( !item.constellation.isEmpty() ) {
            obl.insert ( "constellation", item.constellation );
        }
        if ( !item.jd.isEmpty() ) {
            obl.insert ( "jd", item.jd );
        }
        if ( !item.location.isEmpty() ) {
            obl.insert ( "location", item.location );
        }
        if ( item.fov > 0.0 ) {
            obl.insert ( "fov", item.fov );
        }
        //TODO: check if thi IF is useful
        if ( item.isVisibleMarker ) {
            obl.insert ( "isVisibleMarker", item.isVisibleMarker );
        }

        observingListDataList.insert ( i.key(), obl );
    }

    QVariantMap oblList;
    oblList.insert ( ui->nameOfListLineEdit->text(), observingListDataList );

    //Convert the tree to JSON
    StelJsonParser::write ( oblList, &jsonFile );
    jsonFile.flush();
    jsonFile.close();


}


/*
 * Slot for button obsListRemoveObjectButton
*/
void ObsListCreateEditDialog::obsListRemoveObjectButtonPressed()
{
    //TODO
}

/*
 * Slot for button obsListExportListButton
*/
void ObsListCreateEditDialog::obsListExportListButtonPressed()
{
    //TODO
}

/*
 * Slot for button obsListImportListButton
*/
void ObsListCreateEditDialog::obsListImportListButtonPresssed()
{
    //TODO
}

/*
 * Slot for button obsListSaveButton
*/
void ObsListCreateEditDialog::obsListSaveButtonPressed()
{
    // generat the uuid of the list
    const QString uuidList = QUuid::createUuid().toString();
}

/*
 * Slot for button obsListExitButton
*/
void ObsListCreateEditDialog::obsListExitButtonPressed()
{
    this->close();
    emit exitButtonClicked();
}


/*
 * Load the observing liste in case of edit mode
*/
void ObsListCreateEditDialog::loadObservingList()
{
}


/*
 * Destructor of singleton
*/
void ObsListCreateEditDialog::kill()
{
    if ( m_instance != nullptr ) {
        delete m_instance;
        m_instance = nullptr;
    }
}







