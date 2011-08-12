#ifndef CHTEXTEDIT_H
#define CHTEXTEDIT_H


#include <Qsci/qsciscintilla.h>

class ChTextEdit : public QsciScintilla
{
    Q_OBJECT

    public:
        ChTextEdit(QWidget *parent = 0);
        void loadFile(const QString &fileName);

    private:
        QString curFile;

        QAction *saveAct;
        QAction *saveAsAct;
        QAction *find;
        QAction *findPrevious;
        QAction *findNext;
        QString searchText;
        QAction *goToLine;

        int lineNumberAreaWidth();
        void setNumberAreaWidth();
        bool saveFile(const QString &fileName);
        void setCurrentFile(const QString &fileName);
        QString strippedName(const QString &fullFileName);

    private slots:
        bool saveAs();
        void openFindDialog();
        void findNextText();
        void findPreviousText();
        void refresh();
        void gotoline();
        void contextMenuEvent( QPoint p );

    public slots:
        bool save();
};

#endif // CHTEXTEDIT_H
