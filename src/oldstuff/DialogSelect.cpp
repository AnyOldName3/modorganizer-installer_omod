#include "DialogSelect.h"

#include <functional>

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDebug>
#include <QImageReader>
#include <QLabel>
#include <QPlainTextEdit>
#include <QRadioButton>
#include <QSplitter>
#include <QStackedWidget>
#include <QVBoxLayout>

#include "MIT-licencedCodeToDoStuff/checkboxwordwrap.h"

// there is no hover signal, the only way to know is to override enterEvent()
//
template <class T, typename... Args>
class HoverableWidget : public T
{
public:
  std::function<void ()> onHover;

  HoverableWidget(Args... args, std::function<void ()> h)
    : T(args...), onHover(std::move(h))
  {
  }

protected:
  void enterEvent(QEvent*) override
  {
    onHover();
  }
};

class FixedAspectRatioImageLabel : public QLabel
{
public:
  void setUnscaledPixmap(const QPixmap& pixmap)
  {
    mUnscaledPixmap = pixmap;
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    rescalePixmap(size());

    copySizeLabel.setPixmap(pixmap);
  }

  QSize sizeHint() const override
  {
    qDebug() << "sizeHint() QLabel::sizeHint() =" << QLabel::sizeHint() << ", mUnscaledPixmap.size()" << mUnscaledPixmap.size();
    if (mUnscaledPixmap.isNull())
    {
      qDebug("Top");
      return QLabel::sizeHint();
    }
    else
    {
      qDebug("Bottom");
      // maybe should add frame border
      return mUnscaledPixmap.size();
      //return copySizeLabel.sizeHint();
    }
  }

  int heightForWidth(int width) const override
  {
    // this ignores the difference between size and contentsRect size
    return mUnscaledPixmap.height() * width / (double)mUnscaledPixmap.width();
  }

protected:
  void resizeEvent(QResizeEvent* resizeEvent) override
  {
    QLabel::resizeEvent(resizeEvent);
    qDebug() << "rect size" << rect().size() << ", frameRect size" << frameRect().size() << ", contentsRect" << contentsRect().size();
    rescalePixmap(contentsRect().size());
  }

private:
  void rescalePixmap(const QSize& size)
  {
    qDebug() << "Resizing to" << size;
    qDebug() << "Margin is" << margin();
    setPixmap(mUnscaledPixmap.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation));
  }

  QPixmap mUnscaledPixmap;
  QLabel copySizeLabel;
};


std::optional<QVector<int>> DialogSelect(
  QWidget* parent, const QString& title, const QVector<QString>& items,
  const QVector<QString>& descriptions, const QVector<QString>& pixmaps,
  bool multiSelect)
{
  QDialog d(parent);
  d.setWindowTitle(title);

  auto* mainLayout = new QVBoxLayout;
  d.setLayout(mainLayout);

  auto* splitter = new QSplitter;
  mainLayout->addWidget(splitter);

  // button box
  auto* buttons = new QDialogButtonBox(
    QDialogButtonBox::Ok|QDialogButtonBox::Cancel);

  QObject::connect(buttons, &QDialogButtonBox::accepted, [&] {
    d.accept();
  });

  QObject::connect(buttons, &QDialogButtonBox::rejected, [&] {
    d.reject();
  });

  mainLayout->addWidget(buttons);


  // left panel
  auto* left = new QWidget;
  auto* leftLayout = new QVBoxLayout(left);
  leftLayout->setContentsMargins(0, 0, 0, 0);

  auto* stack = new QStackedWidget;
  leftLayout->addWidget(stack);

  // don't put descriptions of pixmaps at all if there aren't any
  const bool hasDescriptions = !descriptions.empty();
  const bool hasPixmaps = !pixmaps.empty();

  // for each description/item
  for (int i=0; i < std::max(descriptions.size(), pixmaps.size()); ++i) {
    auto* panel = new QWidget;
    auto* panelLayout = new QVBoxLayout(panel);
    //panelLayout->setContentsMargins(0, 0, 0, 0);

    if (hasPixmaps) {
      auto* pixmapLabel = new FixedAspectRatioImageLabel;

      pixmapLabel->setFrameStyle(QFrame::StyledPanel);
      pixmapLabel->setLineWidth(1);

      // make it resizable
      //pixmapLabel->setMinimumSize(1, 150);
      //pixmapLabel->setScaledContents(true);
      //pixmapLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

      if (i < pixmaps.size()) {
        // this item has a pixmap
        // Use QImageReader as (despite what the documentation says) QPixmap uses the extension, rather than the file header, to determine the format.
        QImageReader reader(pixmaps[i]);
        reader.setDecideFormatFromContent(true);
        QImage image = reader.read();
        if (image.isNull()) {
          pixmapLabel->setText(QString("failed to load '%1': %2 (code %3)").arg(pixmaps[i], reader.errorString(), QString::number(reader.error())));
        } else {
          QPixmap pixmap = QPixmap::fromImage(image);
          pixmapLabel->setUnscaledPixmap(std::move(pixmap));
        }
      }

      panelLayout->addWidget(pixmapLabel);
    }

    if (hasDescriptions)
    {
      // description under pixmap (if any)
      auto* description = new QPlainTextEdit(descriptions[i]);

      // this puts the description on top if there's no pixmap
      //description->setAlignment(Qt::AlignLeft|Qt::AlignTop);

      //description->setWordWrap(true);

      panelLayout->addWidget(description);
    }

    // if the pixmap is first, it'll take as much space as it can; if the
    // description is first, the AlignTop makes it work anyways
    panelLayout->setStretch(0, 1);

    stack->addWidget(panel);
  }


  // right panel
  auto* right = new QWidget;
  auto* rightLayout = new QVBoxLayout(right);
  rightLayout->setContentsMargins(0, 0, 0, 0);

  // title
  auto* titleLabel = new QLabel(title);
  titleLabel->setWordWrap(true);
  rightLayout->addWidget(titleLabel);

  // callback when hovering
  auto onHover = [&](int i) {
    stack->setCurrentIndex(i);
  };


  // remember buttons to see which were checked
  QVector<QAbstractButton*> itemButtons;

  // for each item
  for (int i=0; i<items.size(); ++i) {
    QAbstractButton* w = nullptr;

    QString labelText = items[i];
    bool checked = false;
    if (labelText.startsWith('|'))
    {
      labelText = labelText.remove(0, 1);
      checked = true;
    }

    if (multiSelect) {
      w = new HoverableWidget<CheckBoxWordWrap, const QString&>(labelText, [=]{ onHover(i); });
    } else {
      w = new HoverableWidget<RadioButtonWordWrap, const QString&>(labelText, [=]{ onHover(i); });
      checked |= i == 0;
    }

    rightLayout->addWidget(w);
    itemButtons.push_back(w);
    w->setChecked(checked);
  }

  // push all the items to the top
  rightLayout->addStretch(1);

  splitter->addWidget(left);
  splitter->addWidget(right);

  // decent initial size
  d.resize(800, 500);

  if (d.exec() != QDialog::Accepted) {
    return {};
  }


  // go through every button, it doesn't matter whether they're checkboxes or
  // radio buttons
  QVector<int> selection;
  for (int i=0; i<itemButtons.size(); ++i) {
    if (itemButtons[i]->isChecked()) {
      selection.push_back(i);
    }
  }

  return selection;
}
