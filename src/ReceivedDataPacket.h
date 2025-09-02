#ifndef RECEIVEDDATAPACKET_H
#define RECEIVEDDATAPACKET_H

// 受信データを格納する構造体です。
struct ReceivedDataPacket {
  int slideVal1;int slideVal2;
  int sld_sw1_1;int sld_sw1_2;int sld_sw2_1;int sld_sw2_2;
  int sld_sw3_1;int sld_sw3_2;int sld_sw4_1;int sld_sw4_2;
  int sw1;int sw2;int sw3;int sw4;int sw5;int sw6;int sw7;int sw8;
};

#endif // RECEIVEDDATAPACKET_H