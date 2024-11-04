#include "reader.hpp"

#include <cmath>
#include <iostream>

#include "crc/crc.hpp"

std::vector<int> ebv_encode(const std::vector<int>& bits) {
  int n_pad = ((bits.size() + 6) / 7 * 7) - bits.size();
  std::vector<int> padded_bits(n_pad, 0);
  padded_bits.insert(padded_bits.end(), bits.begin(), bits.end());

  int n_block = padded_bits.size() / 7;
  std::vector<int> encoded;
  for (int n = 0; n < n_block; ++n) {
    int extension_bit = (n < n_block - 1) ? 1 : 0;
    encoded.push_back(extension_bit);
    encoded.insert(encoded.end(), padded_bits.begin() + n * 7, padded_bits.begin() + (n + 1) * 7);
  }
  return encoded;
}

PulseIntervalEncoder::PulseIntervalEncoder(int samp_rate, int pw_d) : samp_rate(samp_rate), pw_d(pw_d) {
  n_data0 = static_cast<int>(2 * pw_d * 1e-6 * samp_rate);
  n_data1 = static_cast<int>(4 * pw_d * 1e-6 * samp_rate);
  n_pw = static_cast<int>(pw_d * 1e-6 * samp_rate);
  n_delim = static_cast<int>(DELIM_DURATION * 1e-6 * samp_rate);
  n_rtcal = static_cast<int>(6 * pw_d * 1e-6 * samp_rate);

  data0.resize(n_data0, 1);
  std::fill(data0.end() - n_pw, data0.end(), 0);

  data1.resize(n_data1, 1);
  std::fill(data1.end() - n_pw, data1.end(), 0);
}

std::vector<int> PulseIntervalEncoder::preamble(double blf, int dr) {
  int n_trcal = static_cast<int>(dr / blf * samp_rate);
  std::vector<int> delim(n_delim, 0);
  std::vector<int> rt_cal(n_rtcal, 1);
  std::fill(rt_cal.end() - n_pw, rt_cal.end(), 0);

  std::vector<int> tr_cal(n_trcal, 1);
  std::fill(tr_cal.end() - n_pw, tr_cal.end(), 0);

  std::vector<int> result = delim;
  result.insert(result.end(), data0.begin(), data0.end());
  result.insert(result.end(), rt_cal.begin(), rt_cal.end());
  result.insert(result.end(), tr_cal.begin(), tr_cal.end());
  return result;
}

std::vector<int> PulseIntervalEncoder::frame_sync() {
  std::vector<int> delim(n_delim, 0);
  std::vector<int> rt_cal(n_rtcal, 1);
  std::fill(rt_cal.end() - n_pw, rt_cal.end(), 0);

  std::vector<int> result = delim;
  result.insert(result.end(), data0.begin(), data0.end());
  result.insert(result.end(), rt_cal.begin(), rt_cal.end());
  return result;
}

std::vector<int> PulseIntervalEncoder::encode(const std::vector<int>& data) {
  std::vector<int> sig;
  for (int bit : data) {
    if (bit == 0) {
      sig.insert(sig.end(), data0.begin(), data0.end());
    } else {
      sig.insert(sig.end(), data1.begin(), data1.end());
    }
  }
  return sig;
}

RFIDReaderCommand::RFIDReaderCommand(PulseIntervalEncoder* pie) : pie(pie) {}

std::vector<int> RFIDReaderCommand::select(int pointer, uint8_t length, const std::vector<int>& mask, bool trunc,
                                           target_t target, uint8_t action, membank_t mem_bank) {
  std::vector<int> bits = {1, 0, 1, 0};

  switch (target) {
    case target_t::INV_S0:
      bits.insert(bits.end(), {0, 0, 0});
      break;
    case target_t::INV_S1:
      bits.insert(bits.end(), {0, 0, 1});
      break;
    case target_t::INV_S2:
      bits.insert(bits.end(), {0, 1, 0});
      break;
    case target_t::INV_S3:
      bits.insert(bits.end(), {0, 1, 1});
      break;
    case target_t::SL:
      bits.insert(bits.end(), {1, 0, 0});
      break;
  }

  bits.insert(bits.end(), {action & 4, action & 2, action & 1});

  switch (mem_bank) {
    case membank_t::FILE_TYPE:
      bits.insert(bits.end(), {0, 0});
      break;
    case membank_t::EPC:
      bits.insert(bits.end(), {0, 1});
      break;
    case membank_t::TID:
      bits.insert(bits.end(), {1, 0});
      break;
    case membank_t::FILE_0:
      bits.insert(bits.end(), {1, 1});
      break;
  }

  // Pointer bits (EBV encoded)
  std::vector<int> pointer_bits;
  for (int i = 0; i < 32; ++i) {
    pointer_bits.push_back(pointer >> (31 - i) & 1);
  }
  bits.insert(bits.end(), ebv_encode(pointer_bits).begin(), ebv_encode(pointer_bits).end());

  for (int i = 0; i < 8; ++i) {
    bits.push_back(length >> (7 - i) & 1);
  }

  if (mask.size() != length) {
    throw std::invalid_argument("Mask length must match the specified length.");
  }
  bits.insert(bits.end(), mask.begin(), mask.end());

  bits.push_back(trunc ? 1 : 0);

  std::vector<int> crc_bits = crc16(bits);
  bits.insert(bits.end(), crc_bits.begin(), crc_bits.end());

  auto sync_wave = pie->frame_sync();
  auto wave = pie->encode(bits);
  wave.insert(wave.begin(), sync_wave.begin(), sync_wave.end());
  return wave;
}

std::vector<int> RFIDReaderCommand::query(dr_t dr, miller_t m, bool trext, sel_t sel, session_t session,
                                          inventory_t target, int q) {
  std::vector<int> bits = {1, 0, 0, 0};

  bits.push_back((dr == dr_t::DR_64_3) ? 1 : 0);

  switch (m) {
    case miller_t::M1:
      bits.insert(bits.end(), {0, 0});
      break;
    case miller_t::M2:
      bits.insert(bits.end(), {0, 1});
      break;
    case miller_t::M4:
      bits.insert(bits.end(), {1, 0});
      break;
    case miller_t::M8:
      bits.insert(bits.end(), {1, 1});
      break;
  }

  bits.push_back(trext ? 1 : 0);

  switch (sel) {
    case sel_t::ALL:
      bits.insert(bits.end(), {0, 0});
      break;
    case sel_t::NOT_SL:
      bits.insert(bits.end(), {1, 0});
      break;
    case sel_t::SL:
      bits.insert(bits.end(), {1, 1});
      break;
  }

  switch (session) {
    case session_t::S0:
      bits.insert(bits.end(), {0, 0});
      break;
    case session_t::S1:
      bits.insert(bits.end(), {0, 1});
      break;
    case session_t::S2:
      bits.insert(bits.end(), {1, 0});
      break;
    case session_t::S3:
      bits.insert(bits.end(), {1, 1});
      break;
  }

  bits.push_back((target == inventory_t::B) ? 1 : 0);

  // Q bits
  for (int i = 0; i < 4; ++i) {
    bits.push_back(q >> (3 - i) & 1);
  }

  // CRC5
  std::vector<int> crc_bits = crc5(bits);
  bits.insert(bits.end(), crc_bits.begin(), crc_bits.end());

  auto preamble_wave = pie->preamble(40000, (dr == dr_t::DR_8) ? 8 : 64 / 3);
  auto wave = pie->encode(bits);
  wave.insert(wave.begin(), preamble_wave.begin(), preamble_wave.end());
  return wave;
}

std::vector<int> RFIDReaderCommand::query_rep(session_t session) {
  std::vector<int> bits = {0, 0};

  switch (session) {
    case session_t::S0:
      bits.insert(bits.end(), {0, 0});
      break;
    case session_t::S1:
      bits.insert(bits.end(), {0, 1});
      break;
    case session_t::S2:
      bits.insert(bits.end(), {1, 0});
      break;
    case session_t::S3:
      bits.insert(bits.end(), {1, 1});
      break;
  }

  auto sync_wave = pie->frame_sync();
  auto wave = pie->encode(bits);
  wave.insert(wave.begin(), sync_wave.begin(), sync_wave.end());
  return wave;
}

std::vector<int> RFIDReaderCommand::query_adjust(session_t session, updn_t updn) {
  std::vector<int> bits = {1, 0, 0, 1};

  switch (session) {
    case session_t::S0:
      bits.insert(bits.end(), {0, 0});
      break;
    case session_t::S1:
      bits.insert(bits.end(), {0, 1});
      break;
    case session_t::S2:
      bits.insert(bits.end(), {1, 0});
      break;
    case session_t::S3:
      bits.insert(bits.end(), {1, 1});
      break;
  }

  switch (updn) {
    case updn_t::UNCHANGED:
      bits.insert(bits.end(), {0, 0, 0});
      break;
    case updn_t::INCREACE:
      bits.insert(bits.end(), {1, 1, 0});
      break;
    case updn_t::DECREASE:
      bits.insert(bits.end(), {0, 1, 1});
      break;
  }

  auto sync_wave = pie->frame_sync();
  auto wave = pie->encode(bits);
  wave.insert(wave.begin(), sync_wave.begin(), sync_wave.end());
  return wave;
}

std::vector<int> RFIDReaderCommand::ack(const std::vector<int>& rn16) {
  std::vector<int> bits = {0, 1};

  bits.insert(bits.end(), rn16.begin(), rn16.end());

  auto sync_wave = pie->frame_sync();
  auto wave = pie->encode(bits);
  wave.insert(wave.begin(), sync_wave.begin(), sync_wave.end());
  return wave;
}
