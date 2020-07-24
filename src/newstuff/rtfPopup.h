using namespace cli;

#include <QMainWindow>
#include <QCloseEvent>

class RtfPopup : public QMainWindow
{
  Q_OBJECT

public:
  RtfPopup(System::String^ rtfText, QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

signals:
  void closed();

protected:
  void closeEvent(QCloseEvent* event);
};
