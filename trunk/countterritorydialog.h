#ifndef COUNTTERRITORYDIALOG_H
#define COUNTTERRITORYDIALOG_H

#include <QtGui/QDialog>
#include <QDebug>

namespace Ui {
    class CountTerritoryDialog;
}

class CountTerritoryDialog : public QDialog {
    Q_OBJECT
public:
    CountTerritoryDialog(QWidget *parent = 0);
    ~CountTerritoryDialog();

    void setScoreText(const QString& s);

signals:
    void dialogClosed();

protected:
    virtual void changeEvent(QEvent *e);
    virtual void done( int r );

private:
    Ui::CountTerritoryDialog *m_ui;
};

#endif // COUNTTERRITORYDIALOG_H
