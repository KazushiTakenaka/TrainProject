#ifndef SANEDATAPACKET_H
#define SANEDATAPACKET_H

// 送信データを格納する構造体です。
// ※コントローラー側の受信するデータ構造体と同じにする必要があります。
struct SaneDataPacket {
  int val1;int val2;int val3;int val4;int val5;
};

#endif // SANEDATAPACKET_H