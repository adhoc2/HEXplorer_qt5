#ifndef CHTEXTEDIT_H
#define CHTEXTEDIT_H

#include <QTextEdit>
#include <Qsci/qsciscintilla.h>

class ChTextEdit : public QTextEdit
{
    Q_OBJECT

    public:
        ChTextEdit();
        void loadFile(const QString &fileName);

    private:
        QString curFile;


};

#endif // CHTEXTEDIT_H
