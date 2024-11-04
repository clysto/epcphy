#pragma once

#include <QtWidgets>
#include <vector>

enum class command_t { SELECT, QUERY, QUERY_REP, QUERY_ADJUST, ACK };

class CommandOptionsWidget : public QWidget {
 public:
  CommandOptionsWidget(QWidget* parent = nullptr) : QWidget(parent) {}
  virtual std::vector<int> generate_signal() = 0;
};

class SelectOptionsWidget : public CommandOptionsWidget {
 private:
  QComboBox* target_input;
  QComboBox* action_input;
  QComboBox* mem_bank_input;
  QLineEdit* pointer_input;
  QLineEdit* length_input;
  QLineEdit* mask_input;
  QCheckBox* trunc_input;

 public:
  SelectOptionsWidget(QWidget* parent = nullptr);

  std::vector<int> generate_signal() override;
};

class QueryOptionsWidget : public CommandOptionsWidget {
 private:
  QComboBox* dr_input;
  QComboBox* miller_input;
  QCheckBox* trext_input;
  QComboBox* sel_input;
  QComboBox* session_input;
  QComboBox* target_input;
  QLineEdit* q_input;

 public:
  QueryOptionsWidget(QWidget* parent = nullptr);

  std::vector<int> generate_signal() override;
};

class QueryRepOptionsWidget : public CommandOptionsWidget {
 private:
  QComboBox* session_input;

 public:
  QueryRepOptionsWidget(QWidget* parent = nullptr);

  std::vector<int> generate_signal() override;
};

class QueryAdjustOptionsWidget : public CommandOptionsWidget {
 private:
  QComboBox* session_input;
  QComboBox* updn_input;

 public:
  QueryAdjustOptionsWidget(QWidget* parent = nullptr);

  std::vector<int> generate_signal() override;
};

class AckOptionsWidget : public CommandOptionsWidget {
 private:
  QLineEdit* rn16_input;

 public:
  AckOptionsWidget(QWidget* parent = nullptr);

  std::vector<int> generate_signal() override;
};

class MainWindow : public QMainWindow {
 private:
  QWidget* central_widget;
  QComboBox* command_input;
  QPushButton* generate_btn;
  QVBoxLayout* options_widget_wrapper;
  QVector<QString> command_names = {"Select", "Query", "QueryRep", "QueryAdjust", "Ack"};
  QVector<CommandOptionsWidget*> command_option_widgets;

 public:
  MainWindow();

 public slots:
  void generate();
  void handle_command_change(int index);
};

QVector<int> hex_to_bits(const QString& hex);
QVector<int> bin_to_bits(const QString& bin);
