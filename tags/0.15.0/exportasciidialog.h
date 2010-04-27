#ifndef EXPORTASCIIDIALOG_H
#define EXPORTASCIIDIALOG_H

#include <QtGui/QDialog>
#include "boardwidget.h"

namespace Ui {
    class ExportAsciiDialog;
}

class ExportAsciiDialog : public QDialog {
    Q_OBJECT
public:
    ExportAsciiDialog(QWidget *parent, const BoardWidget::BoardBuffer& buffer);
    ~ExportAsciiDialog();

    void createAscii(int index);
    void createEnglishAscii();
    void createJapaneseAscii();
    bool isStar(int x, int y);

public slots:
    virtual void accept();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::ExportAsciiDialog *m_ui;
    const BoardWidget::BoardBuffer& boardBuffer;

private slots:
    void on_typeComboBox_currentIndexChanged(int index);
};

#endif // EXPORTASCIIDIALOG_H
