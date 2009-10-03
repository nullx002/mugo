#ifndef APPDEF_H
#define APPDEF_H


#define APPNAME "mugo"
#define VERSION "0.14.1"
#define AUTHOR   "nsase"

#define SGF_LINEWIDTH 60

// default value
#define WIN_W 800
#define WIN_H 600
#define BOARD_COLOR QColor(255, 200, 100)
#define WHITE_COLOR QColor(255, 255, 255)
#define BLACK_COLOR QColor(0, 0, 0)
#define BG_COLOR QColor(255, 255, 255)
#define BG_TUTOR_COLOR QColor(85, 175, 127)
#define FOCUS_WHITE_COLOR QColor(255, 0, 0)
#define FOCUS_BLACK_COLOR QColor(255, 0, 0)
#define BRANCH_COLOR QColor(00, 0, 255)
#define SAVE_NAME "%DT%_%PB%_%PW%"


#include <QFileDialog>
QString getOpenFileName(QWidget* parent = 0, const QString& caption = QString(), const QString& dir = QString(), const QString& filter = QString(), QString* selectedFilter = 0, QFileDialog::Options options = 0);
QString getSaveFileName(QWidget* parent = 0, const QString& caption = QString(), const QString& dir = QString(), const QString& filter = QString(), QString* selectedFilter = 0, QFileDialog::Options options = 0);


#if defined(Q_WS_WIN)
#   define strcasecmp _stricmp
#endif


#endif // APPDEF_H
