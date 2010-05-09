#ifndef SAVEIMAGEDIALOG_H
#define SAVEIMAGEDIALOG_H

#include <QtGui/QDialog>
#include <QFileInfo>

namespace Ui {
    class SaveImageDialog;
}

/**
* export as image dialog
*/
class SaveImageDialog : public QDialog {
    Q_OBJECT
public:
    SaveImageDialog(QWidget *parent = 0);
    ~SaveImageDialog();

    virtual void accept();

    QFileInfo fileInfo;
    int       imageSize;
    bool      showCoordinate;
    bool      monochrome;

protected:
    void changeEvent(QEvent *e);

private:
    Ui::SaveImageDialog *m_ui;

private slots:
    void on_fileNameEdit_textChanged(QString );
    void on_fileBrowseButton_clicked();
};

#endif // SAVEIMAGEDIALOG_H
