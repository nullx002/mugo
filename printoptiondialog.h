#ifndef PRINTOPTIONDIALOG_H
#define PRINTOPTIONDIALOG_H

#include <QtGui/QDialog>

namespace Ui {
    class PrintOptionDialog;
}

class PrintOptionDialog : public QDialog {
    Q_OBJECT
public:
    PrintOptionDialog(QWidget *parent = 0);
    ~PrintOptionDialog();

    int printOption() const{ return printOption_; }
    int movesPerPage() const{ return movesPerPage_; }
    bool showCoordinate() const{ return showCoordinate_; }
    const QFont& font() const{ return font_; }
    const QString& headerLeftFormat() const{ return headerLeftFormat_; }
    const QString& headerCenterFormat() const{ return headerCenterFormat_; }
    const QString& headerRightFormat() const{ return headerRightFormat_; }
    const QString& footerLeftFormat() const{ return footerLeftFormat_; }
    const QString& footerCenterFormat() const{ return footerCenterFormat_; }
    const QString& footerRightFormat() const{ return footerRightFormat_; }

protected:
    void changeEvent(QEvent *e);

public slots:
    virtual void accept();

private:
    Ui::PrintOptionDialog *m_ui;
    int printOption_;
    int movesPerPage_;
    bool  showCoordinate_;
    QFont font_;
    QString headerLeftFormat_;
    QString headerCenterFormat_;
    QString headerRightFormat_;
    QString footerLeftFormat_;
    QString footerCenterFormat_;
    QString footerRightFormat_;
};

#endif // PRINTOPTIONDIALOG_H
