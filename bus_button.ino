/*
 * バスの降車ボタンのオモチャ　赤外線リモコン付き  2018/12/24 7m4mon
 * バスの後車ボタンが押されたら、電源を握りに行ってアナウンスを再生、ついでに赤外線リモコンコードも送出する。
 * 再生中に再度ボタンが押されたら赤外線リモコンコードを送出する。
 * 再生終了後は電源を離して電源断待ちとなるが、再度ボタンが押されたら復活してアナウンスを再生する。
 * 
 * 最大30,720バイトのフラッシュメモリのうち、スケッチが4,208バイト（13%）を使っています。
 * 最大2,048バイトのRAMのうち、グローバル変数が478バイト（23%）を使っていて、ローカル変数で1,570バイト使うことができます。
 */

#include <DFPlayer_Mini_Mp3.h>
#include <IRremote.h>

#define PIN_SEL_SW     11
#define PIN_MP3_BUSY   12       // MP3モジュールが再生中の時 Low
#define PIN_LAMP       2        // 降車ボタンのランプ
#define PIN_POW_SW     10       // 降車ボタンが押されていることを検出する。
#define PIN_POW_ON     13       // 自分の電源を握りに行く出力ピン
#define PIN_IR_LED     3

#define TIMEOUT        15000    // MAX 65535 = 65.535秒

#define SHARP_TV_ADDR  0x555A
#define SHARP_TV_CHUP  0xF1488885
#define SHARP_TV_CHDN  0xF1484889

uint8_t  sw_pos;
uint16_t tot;

IRsend irsend;

void det_sw_pos(){
    sw_pos =  digitalRead(PIN_SEL_SW);
    sw_pos += 1;                        //MP3再生のファイル名が１から始まるので。
}

void send_ir(){
    uint32_t ir_code;
    det_sw_pos();
    switch (sw_pos){
        case 1:
            ir_code = SHARP_TV_CHDN;
        break;
        case 2:
            ir_code = SHARP_TV_CHUP;
        break;
        default:
            ir_code = 0;
        break;
    }
    irsend.sendPanasonic(SHARP_TV_ADDR,ir_code); 
    delay(10);
    irsend.sendPanasonic(SHARP_TV_ADDR,ir_code);
}

void play_sound(){
    det_sw_pos();
    mp3_play (sw_pos);
    send_ir();
    tot = 0;
    //再生終了またはタイムアウトで終了。ただし最初に3秒待つ。
    while(((digitalRead(PIN_MP3_BUSY) == false && (tot < TIMEOUT)) ||
            (tot < 3000))){
        if(digitalRead(PIN_POW_SW) == false){
            /* 再生中にボタンが押されたときはリモコンコードを送出する */
            while(digitalRead(PIN_POW_SW) == false){;}  //ボタンが離されるのを待つ
            send_ir();
            delay(100);     // チャタリング防止
            tot += 110;
        }
        delay(1);
        tot++;
    }
}

void setup () {
    //とりあえず、電源を握りにいく
    digitalWrite(PIN_POW_ON, HIGH);
    digitalWrite(PIN_LAMP,   HIGH);
    pinMode(PIN_POW_SW, INPUT_PULLUP);
    pinMode(PIN_SEL_SW, INPUT_PULLUP);
    pinMode(PIN_IR_LED, OUTPUT );
    delay (300); //DFPlayer起動待ち
    
    Serial.begin (9600);
    mp3_set_serial (Serial);    //set Serial for DFPlayer-mini mp3 module 

    play_sound();
    //再生終了またはタイムアウトで電源断待ち。
    digitalWrite(PIN_LAMP, LOW);
    digitalWrite(PIN_POW_ON, LOW);
}

/* 再生終了後、電源断待ちの時にボタンが押されたら復活する。*/
void loop() {
    if(digitalRead(PIN_POW_SW) == false){
        digitalWrite(PIN_POW_ON, HIGH);
        digitalWrite(PIN_LAMP,   HIGH);
        play_sound();
    }
    digitalWrite(PIN_LAMP, LOW);
    digitalWrite(PIN_POW_ON, LOW);
}
