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

#ifndef BOOKMARKSLOCATIONSDIALOG_H
#define BOOKMARKSLOCATIONSDIALOG_H

#include <QObject>
#include <QMap>
#include <QUuid>
#include <QStandardItemModel>

#include "StelDialog.hpp"

class Ui_bookmarksLocationsDialogForm;

//Data for a bookmark location
struct bookmarkLocation
{
	QString name; //the name of the location. Ex: New York, United States
	QString latitude; 
	QString longitude;
	QString jd; //date and time in Julian Day
};
Q_DECLARE_METATYPE(bookmarkLocation)

/**
 * Class for the bookmarks locations dialog
 */
class BookmarksLocationsDialog :  public StelDialog
{
    
    Q_OBJECT
    
public:
	BookmarksLocationsDialog(QObject* parent);
	virtual ~BookmarksLocationsDialog();
    
    //! Notify that the application style changed
	void styleChanged();
    
protected:
    //! Initialize the dialog widgets and connect the signals/slots.
    virtual void createDialogContent();
    
    Ui_bookmarksLocationsDialogForm *ui;
        
private:
    static constexpr const char* FILE_NAME = "bookmarks_locations.json";
    
    enum BookmarksLocationsColumns {
		ColumnUUID,	//! 0 - UUID of bookmark
		ColumnName,	//! 1 - name or designation of object
		ColumnDate,	//! 2 - date and time (optional)
		ColumnLatitude,	//! 3 - latitude
        ColumnLongitude, //! 4 - longitude
		ColumnCount	//! 5 - total number of columns
	};

    class StelCore* core;
	class StelObjectMgr* objectMgr;
	class LabelMgr* labelMgr;
    
    //Model for displaying bookmarks in the dialog (bookmarksLocationsTreeView)
    QStandardItemModel * bookmarksLocationsListModel;
    
    //Path for the json bookmarks file
    QString bookmarksLocationsJsonPath;
    
    //HashMap to store the bookmarks
	QHash<QString, bookmarkLocation> bookmarksCollection;
    
    //Define the header name for bookmarks locations table
    void setBookmarksLocationsHeaderNames();
    
    //Add model raw in the model list (bookmarksLocationsListModel)
    void addModelRow(int number, QString uuid, QString name, QString date, QString latitude, QString longitude);
    
    //Save bookmarks in Json file
    void saveBookmarks() const;
    
public slots:
        void retranslate();
    
private slots:
    void addBookmarkCurrentLocationButtonPressed();
    void addBookmarkLocationFromListButtonPressed();
    void addBookmarkLocationFromGpsCoordButtonPressed();
    void goToLocationButtonPressed();
    void importBookmarksLocationsButtonPressed();
    void exportBookmarksLocationsButtonPressed();
    void removeBookmarkLocationButtonPressed();
    void clearBookmarksLocationsButtonPressed();
    
    void selectCurrentBookmarkLocation(const QModelIndex &modelIdx);
};

#endif // BOOKMARKSLOCATIONSDIALOG_H
