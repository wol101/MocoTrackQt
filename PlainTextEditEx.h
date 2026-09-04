#pragma once

#include <QPlainTextEdit>

class PlainTextEditEx : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit PlainTextEditEx(QWidget *parent = nullptr);

public slots:
    void saveAs();
    void clearContents();

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
};
