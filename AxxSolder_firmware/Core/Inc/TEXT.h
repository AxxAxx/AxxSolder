//TEXT.h
//Собщения на экране
#ifndef __text_H
#define __text_H



extern const char *t_language[2];

extern char *t_Yes[2];// {"Так, "Yes "};
extern char *t_No[2];// {"Нi   ", "No   "};




extern char *t_time[2];//     = {"          ЧАС", "   STANDBY"};
extern char *t_passed[2];//   = {"     СПЛИНУВ",  "   TIMEOUT"};

extern char *t_low_U[2];//      = {"    НИЗЬКА", " INP. VOLTAGE"};
extern char *t_voltage[2];//  = {"    НАПРУГА", "     TOO LOW"};

extern char *t_low_P[2];//  = {"    НИЗЬКА", " INP. POWER"};
extern char *t_power[2];//  = {" ПОТУЖНІСТЬ", "     TOO LOW"};

extern char *t_cartridge[2];//  = {"АВАРIЯ  ЖАЛА", "     NO TIP"};
extern char *t_emergency_I[2];//  = {"ПО СТРУМУ", " EMERGENCY"};


extern char *t_overtemp[2];//  = {"    ПЕРЕГРIВ", "  OVERTEMP"};


extern char *t_cartridge_IT[2];//  = {"КАРТРИДЖ", "     NO TIP"};
extern char *t_emergency_IT[2];//  = {"ВIДСУТНIЙ", "DETECTED"};


extern char *t_emergency_dT[2];//  = {"ПО ДЕЛЬТI ТЕМП", "OR FAULTY TIP"};

extern char *t_presets[2];// = {"ПPECETИ", "PRESETS"};

extern char *t_set_temp[2];// = {"Heoбx. тeмn.", "Set temp"};

extern char *t_actual_temp[2];// = {"Peaл. тeмn.", "Actual temp"};

extern char *t_W[2];// = {"Bт", "W"};

extern char *t_V[2];// = {"B", "V"};

extern char *t_type[2];// = {"Tuп:    ", "Type:  "};

extern char *t_src[2];// = {"Жuвл.:", "SRC:"};

extern char *t_Noname[2];// = {"Xтoзнa  ", "No name"};


#endif /* __text_H */

