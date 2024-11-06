#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

const int DELIM_DURATION = 12;

enum class target_t { INV_S0, INV_S1, INV_S2, INV_S3, SL };
enum class inventory_t { A, B };
enum class membank_t { FILE_TYPE, EPC, TID, FILE_0 };
enum class session_t { S0, S1, S2, S3 };
enum class dr_t { DR_8, DR_64_3 };
enum class sel_t { ALL, SL, NOT_SL };
enum class updn_t { UNCHANGED, INCREACE, DECREASE };
enum class miller_t { M1, M2, M4, M8 };

std::vector<int> ebv_encode(const std::vector<int>& bits);

class PulseIntervalEncoder {
 public:
  PulseIntervalEncoder(int samp_rate, int pw_d = 12);
  std::vector<int> preamble(double blf = 40000, int dr = 8);
  std::vector<int> frame_sync();
  std::vector<int> encode(const std::vector<int>& data);

 private:
  int samp_rate;
  int pw_d;
  int n_data0;
  int n_data1;
  int n_pw;
  int n_delim;
  int n_rtcal;
  std::vector<int> data0;
  std::vector<int> data1;
};

class RFIDReaderCommand {
 public:
  RFIDReaderCommand(PulseIntervalEncoder* pie);
  std::vector<int> select(int pointer, uint8_t length, const std::vector<int>& mask, bool trunc = false,
                          target_t target = target_t::SL, uint8_t action = 0,
                          membank_t mem_bank = membank_t::FILE_TYPE);
  std::vector<int> query(dr_t dr = dr_t::DR_8, miller_t m = miller_t::M1, bool trext = false, sel_t sel = sel_t::ALL,
                         session_t session = session_t::S0, inventory_t target = inventory_t::A, int q = 0);
  std::vector<int> query_rep(session_t session = session_t::S0);
  std::vector<int> query_adjust(session_t session = session_t::S0, updn_t updn = updn_t::UNCHANGED);
  std::vector<int> ack(const std::vector<int>& rn16);

 private:
  PulseIntervalEncoder* pie;
};
