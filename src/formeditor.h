#ifndef FORMEDITOR_H
#define FORMEDITOR_H

#include <QtGui/QWidget>

namespace Ui {
    class FormEditor;
}

class FormEditor : public QWidget {
    Q_OBJECT
public:
    FormEditor(QWidget *parent = 0);
    ~FormEditor();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::FormEditor *m_ui;
};

#endif // FORMEDITOR_H
