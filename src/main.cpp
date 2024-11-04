#include "gui.hpp"
#include "reader.hpp"

int main(int argc, char* argv[]) {
  auto app = new QApplication(argc, argv);
  auto window = MainWindow{};
  window.show();
  return app->exec();
}
