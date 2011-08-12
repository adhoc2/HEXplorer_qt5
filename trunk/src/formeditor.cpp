#include "formeditor.h"
#include "ui_formeditor.h"

FormEditor::FormEditor(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::FormEditor)
{
    m_ui->setupUi(this);
}

FormEditor::~FormEditor()
{
    delete m_ui;
}

void FormEditor::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
