#include "printoptiondialog.h"
#include "ui_printoptiondialog.h"

PrintOptionDialog::PrintOptionDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::PrintOptionDialog),
    printOption_(0)
{
    m_ui->setupUi(this);
}

PrintOptionDialog::~PrintOptionDialog()
{
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
