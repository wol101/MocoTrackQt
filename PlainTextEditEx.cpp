#include "PlainTextEditEx.h"

#include <QAction>
#include <QContextMenuEvent>
#include <QFile>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QTextCursor>
#include <QTextStream>

PlainTextEditEx::PlainTextEditEx(QWidget *parent)
    : QPlainTextEdit(parent)
{
}

void PlainTextEditEx::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = createStandardContextMenu(event->pos());

    if (isReadOnly()) {
        // Qt's standard menu omits Undo/Redo entirely when read-only, even
        // though undo()/redo() aren't actually blocked by that flag.
        QAction *before = menu->actions().value(0, nullptr);

        QAction *redoAction = new QAction(tr("&Redo"), menu);
        redoAction->setShortcut(QKeySequence::Redo);
        redoAction->setEnabled(document()->isRedoAvailable());
        connect(redoAction, &QAction::triggered, this, &QPlainTextEdit::redo);
        menu->insertAction(before, redoAction);

        QAction *undoAction = new QAction(tr("&Undo"), menu);
        undoAction->setShortcut(QKeySequence::Undo);
        undoAction->setEnabled(document()->isUndoAvailable());
        connect(undoAction, &QAction::triggered, this, &QPlainTextEdit::undo);
        menu->insertAction(redoAction, undoAction);

        QAction *sep = new QAction(menu);
        sep->setSeparator(true);
        menu->insertAction(before, sep);
    }

    menu->addSeparator();

    QAction *saveAsAction = menu->addAction(tr("Save As..."));
    connect(saveAsAction, &QAction::triggered, this, &PlainTextEditEx::saveAs);

    QAction *clearAction = menu->addAction(tr("Clear"));
    clearAction->setEnabled(!document()->isEmpty());
    connect(clearAction, &QAction::triggered, this, &PlainTextEditEx::clearContents);

    menu->exec(event->globalPos());
    delete menu;
}

void PlainTextEditEx::saveAs()
{
    const QString fileName = QFileDialog::getSaveFileName(
        this, tr("Save As"), QString(), tr("Text Files (*.txt);;All Files (*)"));
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Save As"),
            tr("Could not write to file %1:\n%2").arg(fileName, file.errorString()));
        return;
    }

    QTextStream out(&file);
    out << toPlainText();
}

void PlainTextEditEx::clearContents()
{
    // Not clear(): it wipes undo history. Cursor removal keeps Clear itself
    // undoable, works even when the widget is read-only.
    QTextCursor cursor(document());
    cursor.beginEditBlock();
    cursor.select(QTextCursor::Document);
    cursor.removeSelectedText();
    cursor.endEditBlock();
}
