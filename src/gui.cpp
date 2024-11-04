#include "gui.hpp"

#include <complex>
#include <fstream>

#include "reader.hpp"

void dump_file(const std::vector<int>& data, const char* path) {
  std::ofstream file(path, std::ios::binary);
  for (int i = 0; i < data.size(); i++) {
    auto sample = std::complex<float>(data[i], 0);
    file.write(reinterpret_cast<const char*>(&sample), sizeof(sample));
  }
  file.close();
}

MainWindow::MainWindow() : QMainWindow() {
  central_widget = new QWidget(this);
  setCentralWidget(central_widget);
  setWindowTitle("RFID Reader Command Generator");

  auto vbox = new QVBoxLayout(central_widget);
  command_input = new QComboBox(central_widget);
  for (auto& name : command_names) {
    command_input->addItem(name);
  }

  options_widget_wrapper = new QVBoxLayout();
  command_option_widgets = {new SelectOptionsWidget(), new QueryOptionsWidget(), new QueryRepOptionsWidget(),
                            new QueryAdjustOptionsWidget(), new AckOptionsWidget()};
  generate_btn = new QPushButton("Generate", central_widget);
  vbox->addWidget(command_input);
  vbox->addLayout(options_widget_wrapper);
  vbox->addStretch();

  vbox->addWidget(generate_btn);
  connect(generate_btn, &QPushButton::clicked, this, &MainWindow::generate);
  connect(command_input, &QComboBox::currentIndexChanged, this, &MainWindow::handle_command_change);
  handle_command_change(0);
  central_widget->setMinimumWidth(400);
}

void MainWindow::generate() {
  qDebug() << "Generating command...";
  QFileDialog dialog(this);
  dialog.setFileMode(QFileDialog::AnyFile);
  dialog.setAcceptMode(QFileDialog::AcceptSave);
  dialog.setDefaultSuffix("cf32");
  dialog.setViewMode(QFileDialog::Detail);
  dialog.setDirectory(QDir::homePath());
  dialog.setWindowTitle("Save Generated Signal");

  if (dialog.exec()) {
    auto file = dialog.selectedFiles().first();
    auto index = command_input->currentIndex();
    auto widget = command_option_widgets[index];
    auto signal = widget->generate_signal();
    dump_file(signal, file.toStdString().c_str());
    QMessageBox msgBox;
    msgBox.setText("Signal generated successfully!");
    msgBox.exec();
  }
}

void MainWindow::handle_command_change(int index) {
  qDebug() << "Command changed to: " << command_input->itemText(index);
  auto widget = command_option_widgets[index];

  if (options_widget_wrapper->count() > 0) {
    auto prevWidget = options_widget_wrapper->itemAt(0)->widget();
    prevWidget->setParent(nullptr);
    options_widget_wrapper->removeItem(options_widget_wrapper->itemAt(0));
  }

  if (widget) {
    options_widget_wrapper->addWidget(widget, 0, Qt::AlignLeft);
  }

  update();
};

SelectOptionsWidget::SelectOptionsWidget(QWidget* parent) : CommandOptionsWidget(parent) {
  auto layout = new QFormLayout(this);
  target_input = new QComboBox(this);
  action_input = new QComboBox(this);
  mem_bank_input = new QComboBox(this);
  pointer_input = new QLineEdit(this);
  length_input = new QLineEdit(this);
  mask_input = new QLineEdit(this);
  trunc_input = new QCheckBox(this);

  target_input->addItem("INV_S0", QVariant::fromValue(target_t::INV_S0));
  target_input->addItem("INV_S1", QVariant::fromValue(target_t::INV_S1));
  target_input->addItem("INV_S2", QVariant::fromValue(target_t::INV_S2));
  target_input->addItem("INV_S3", QVariant::fromValue(target_t::INV_S3));
  target_input->addItem("SL", QVariant::fromValue(target_t::SL));

  action_input->addItem("000", QVariant::fromValue(0));
  action_input->addItem("001", QVariant::fromValue(1));
  action_input->addItem("010", QVariant::fromValue(2));
  action_input->addItem("011", QVariant::fromValue(3));
  action_input->addItem("100", QVariant::fromValue(4));
  action_input->addItem("101", QVariant::fromValue(5));
  action_input->addItem("110", QVariant::fromValue(6));
  action_input->addItem("111", QVariant::fromValue(7));

  mem_bank_input->addItem("FILE_TYPE", QVariant::fromValue(membank_t::FILE_TYPE));
  mem_bank_input->addItem("EPC", QVariant::fromValue(membank_t::EPC));
  mem_bank_input->addItem("TID", QVariant::fromValue(membank_t::TID));
  mem_bank_input->addItem("FILE_0", QVariant::fromValue(membank_t::FILE_0));

  layout->addRow("Target", target_input);
  layout->addRow("Action", action_input);
  layout->addRow("Memory Bank", mem_bank_input);
  layout->addRow("Pointer", pointer_input);
  layout->addRow("Length", length_input);
  layout->addRow("Mask", mask_input);
  layout->addRow("Truncate", trunc_input);
}

std::vector<int> SelectOptionsWidget::generate_signal() {
  auto pie = PulseIntervalEncoder{2000000};
  auto reader = RFIDReaderCommand{&pie};
  auto pointer = pointer_input->text().toInt();
  auto length = length_input->text().toInt();
  auto mask_ = hex_to_bits(mask_input->text());
  auto mask = std::vector<int>(mask_.begin(), mask_.end());
  auto trunc = trunc_input->isChecked();
  auto target = target_input->currentData().value<target_t>();
  auto action = action_input->currentData().value<uint8_t>();
  auto mem_bank = mem_bank_input->currentData().value<membank_t>();
  return reader.select(pointer, length, mask, trunc, target, action, mem_bank);
}

QueryOptionsWidget::QueryOptionsWidget(QWidget* parent) : CommandOptionsWidget(parent) {
  auto layout = new QFormLayout(this);
  dr_input = new QComboBox(this);
  miller_input = new QComboBox(this);
  trext_input = new QCheckBox(this);
  sel_input = new QComboBox(this);
  session_input = new QComboBox(this);
  target_input = new QComboBox(this);
  q_input = new QLineEdit(this);

  dr_input->addItem("8", QVariant::fromValue(dr_t::DR_8));
  dr_input->addItem("64/3", QVariant::fromValue(dr_t::DR_64_3));

  miller_input->addItem("M1 (FM0)", QVariant::fromValue(miller_t::M1));
  miller_input->addItem("M2", QVariant::fromValue(miller_t::M2));
  miller_input->addItem("M4", QVariant::fromValue(miller_t::M4));
  miller_input->addItem("M8", QVariant::fromValue(miller_t::M8));

  sel_input->addItem("ALL", QVariant::fromValue(sel_t::ALL));
  sel_input->addItem("SL", QVariant::fromValue(sel_t::SL));
  sel_input->addItem("NOT_SL", QVariant::fromValue(sel_t::NOT_SL));

  session_input->addItem("S0", QVariant::fromValue(session_t::S0));
  session_input->addItem("S1", QVariant::fromValue(session_t::S1));
  session_input->addItem("S2", QVariant::fromValue(session_t::S2));
  session_input->addItem("S3", QVariant::fromValue(session_t::S3));

  target_input->addItem("A", QVariant::fromValue(inventory_t::A));
  target_input->addItem("B", QVariant::fromValue(inventory_t::B));

  layout->addRow("DR", dr_input);
  layout->addRow("Miller", miller_input);
  layout->addRow("Trext", trext_input);
  layout->addRow("Sel", sel_input);
  layout->addRow("Session", session_input);
  layout->addRow("Target", target_input);
  layout->addRow("Q", q_input);
}

std::vector<int> QueryOptionsWidget::generate_signal() {
  auto pie = PulseIntervalEncoder{2000000};
  auto reader = RFIDReaderCommand{&pie};
  auto dr = dr_input->currentData().value<dr_t>();
  auto miller = miller_input->currentData().value<miller_t>();
  auto trext = trext_input->isChecked();
  auto sel = sel_input->currentData().value<sel_t>();
  auto session = session_input->currentData().value<session_t>();
  auto target = target_input->currentData().value<inventory_t>();
  auto q = q_input->text().toInt();
  return reader.query(dr, miller, trext, sel, session, target, q);
}

QueryRepOptionsWidget::QueryRepOptionsWidget(QWidget* parent) : CommandOptionsWidget(parent) {
  auto layout = new QFormLayout(this);
  session_input = new QComboBox(this);

  session_input->addItem("S0", QVariant::fromValue(session_t::S0));
  session_input->addItem("S1", QVariant::fromValue(session_t::S1));
  session_input->addItem("S2", QVariant::fromValue(session_t::S2));
  session_input->addItem("S3", QVariant::fromValue(session_t::S3));

  layout->addRow("Session", session_input);
}

std::vector<int> QueryRepOptionsWidget::generate_signal() {
  auto pie = PulseIntervalEncoder{2000000};
  auto reader = RFIDReaderCommand{&pie};
  auto session = session_input->currentData().value<session_t>();
  return reader.query_rep(session);
}

QueryAdjustOptionsWidget::QueryAdjustOptionsWidget(QWidget* parent) : CommandOptionsWidget(parent) {
  auto layout = new QFormLayout(this);
  session_input = new QComboBox(this);
  updn_input = new QComboBox(this);

  session_input->addItem("S0", QVariant::fromValue(session_t::S0));
  session_input->addItem("S1", QVariant::fromValue(session_t::S1));
  session_input->addItem("S2", QVariant::fromValue(session_t::S2));
  session_input->addItem("S3", QVariant::fromValue(session_t::S3));

  updn_input->addItem("No change to Q", QVariant::fromValue(updn_t::UNCHANGED));
  updn_input->addItem("Q = Q + 1", QVariant::fromValue(updn_t::INCREACE));
  updn_input->addItem("Q = Q – 1", QVariant::fromValue(updn_t::DECREASE));

  layout->addRow("Session", session_input);
  layout->addRow("UpDn", updn_input);
}

std::vector<int> QueryAdjustOptionsWidget::generate_signal() {
  auto pie = PulseIntervalEncoder{2000000};
  auto reader = RFIDReaderCommand{&pie};
  auto session = session_input->currentData().value<session_t>();
  auto updn = updn_input->currentData().value<updn_t>();
  return reader.query_adjust(session, updn);
}

AckOptionsWidget::AckOptionsWidget(QWidget* parent) : CommandOptionsWidget(parent) {
  auto layout = new QFormLayout(this);
  rn16_input = new QLineEdit(this);
  layout->addRow("RN16", rn16_input);
}

std::vector<int> AckOptionsWidget::generate_signal() {
  auto pie = PulseIntervalEncoder{2000000};
  auto reader = RFIDReaderCommand{&pie};
  auto rn16_ = bin_to_bits(rn16_input->text());
  auto rn16 = std::vector<int>(rn16_.begin(), rn16_.end());
  return reader.ack(rn16);
}

QVector<int> hex_to_bits(const QString& hex) {
  QVector<int> bits;
  QString cleanedHex = hex.simplified().replace(" ", "");
  for (int i = 0; i < cleanedHex.length(); i += 2) {
    QString byteString = cleanedHex.mid(i, 2);
    bool ok;
    int value = byteString.toInt(&ok, 16);

    if (ok) {
      for (int j = 7; j >= 0; --j) {
        bits.append((value >> j) & 1);
      }
    }
  }

  return bits;
}

QVector<int> bin_to_bits(const QString& bin) {
  QVector<int> bits;
  QString cleanedBin = bin.simplified().replace(" ", "");
  for (int i = 0; i < cleanedBin.length(); i++) {
    if (cleanedBin[i] == '0') {
      bits.append(0);
    } else if (cleanedBin[i] == '1') {
      bits.append(1);
    }
  }

  return bits;
}