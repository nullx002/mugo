#include <QDebug>
#include <QSettings>
#include "printoptiondialog.h"
#include "ui_printoptiondialog.h"

PrintOptionDialog::PrintOptionDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::PrintOptionDialog),
    printOption_(0),
    movesPerPage_(0)
{
    m_ui->setupUi(this);

    QSettings settings;

    QRadioButton* printOptions[] = {
        m_ui->printActiveBoard,
        m_ui->printActiveBranch,
        m_ui->printActiveBranchN,
        m_ui->printAllBranches,
        m_ui->printAllBranchesN,
    };
    int N = sizeof(printOptions) / sizeof(printOptions[0]);
    int type = settings.value("print/type").toInt();
    if (type >= 0 && type < N)
        printOptions[type]->setChecked(true);

    int movesPerPage = settings.value("print/movesPerPage", 50).toInt();
    m_ui->activeBranchMovesPerPage->setValue(movesPerPage);
    m_ui->allBranchMovesPerPage->setValue(movesPerPage);
    m_ui->showCoordinate->setChecked( settings.value("print/showCoordinate" , true).toBool() );
    QFont font = settings.value("print/font", QFont()).value<QFont>();
    m_ui->fontComboBox->setCurrentFont(font);
    m_ui->fontSizeComboBox->setCurrentIndex(font.pointSize() - 8);
    m_ui->headerLeftEdit->setText  ( settings.value("print/headerLeft"  , "%GN% %PW%(W) vs %PB%(B)").toString() );
    m_ui->headerCenterEdit->setText( settings.value("print/headerCenter", "").toString() );
    m_ui->headerRightEdit->setText ( settings.value("print/headerRight" , "%datetime%").toString() );
    m_ui->footerLeftEdit->setText  ( settings.value("print/footerLeft"  , "%file%").toString() );
    m_ui->footerCenterEdit->setText( settings.value("print/footerCenter", "").toString() );
    m_ui->footerRightEdit->setText ( settings.value("print/footerRight" , "%page%").toString() );
}

PrintOptionDialog::~PrintOptionDialog()
{
    delete m_ui;
}

void PrintOptionDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void PrintOptionDialog::accept()
{
    QDialog::accept();

    QRadioButton* printOptions[] = {
        m_ui->printActiveBoard,
        m_ui->printActiveBranch,
        m_ui->printActiveBranchN,
        m_ui->printAllBranches,
        m_ui->printAllBranchesN,
    };
    int N = sizeof(printOptions) / sizeof(printOptions[0]);
    for (int i=0; i<N; ++i)
        if (printOptions[i]->isChecked()){
            printOption_ = i;
            break;
        }

    if (printOption_ == 2)
        movesPerPage_ = m_ui->activeBranchMovesPerPage->value();
    else if (printOption_ == 4)
        movesPerPage_ = m_ui->allBranchMovesPerPage->value();
    else
        movesPerPage_ = 0;

    showCoordinate_ = m_ui->showCoordinate->isChecked();
    font_ = m_ui->fontComboBox->currentFont();
    font_.setPointSize( m_ui->fontSizeComboBox->currentText().toInt() );

    headerLeftFormat_   = m_ui->headerLeftEdit->text();
    headerCenterFormat_ = m_ui->headerCenterEdit->text();
    headerRightFormat_  = m_ui->headerRightEdit->text();
    footerLeftFormat_   = m_ui->footerLeftEdit->text();
    footerCenterFormat_ = m_ui->footerCenterEdit->text();
    footerRightFormat_  = m_ui->footerRightEdit->text();

    QSettings settings;
    settings.setValue("print/type", printOption_);
    if (movesPerPage_ != 0)
        settings.setValue("print/movesPerPage", movesPerPage_);
    settings.setValue("print/showCoordinate", showCoordinate_);
    settings.setValue("print/font", font_);
    settings.setValue("print/headerLeft", headerLeftFormat_);
    settings.setValue("print/headerCenter", headerCenterFormat_);
    settings.setValue("print/headerRight", headerRightFormat_);
    settings.setValue("print/footerLeft", footerLeftFormat_);
    settings.setValue("print/footerCenter", footerCenterFormat_);
    settings.setValue("print/footerRight", footerRightFormat_);
}
