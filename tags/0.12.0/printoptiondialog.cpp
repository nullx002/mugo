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


    QSettings settings;
    settings.setValue("print/type", printOption_);
    if (movesPerPage_ != 0)
        settings.setValue("print/movesPerPage", movesPerPage_);
}
