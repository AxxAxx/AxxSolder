#ifndef __APP_COLORS_H
#define __APP_COLORS_H

/* AxxSolder UI palette.
 * Values are the corrected (post-SPI-fix) RGB565 codes that reproduce the original hand-tuned on-screen colours. Names describe the UI role. */

#define C_Background       0x0000   /* screen / text background           */
#define C_Text             0xFFFF   /* primary text, labels, frames       */
#define C_Header           0xFF07   /* title, header lines, countdown     */
#define C_Highlight        0xE907   /* standby/sleep box, popups, edit hl  */
#define C_PowerBar         0x33F4   /* RUN power-bar fill                 */
#define C_Footer           0x6E8C   /* bottom labels + divider lines      */
#define C_Warning          0xC007   /* warning / selected menu line       */
#define C_MenuDim          0xB4D6   /* "Back" / secondary menu text       */
#define C_GraphTemp        0x25EB   /* temperature trace (RUN)            */
#define C_GraphAxis        0xBDF7   /* set-temp trace + time axis         */
#define C_GraphPercent     0x00F8   /* percent text                       */
#define C_GraphPower       0x3F00   /* power-% text + temp trace (SLEEP)  */
#define C_GraphGrid        0x6AAD   /* dashed grid                        */
#define C_GraphPowerSleep  0xFE03   /* power trace (SLEEP)                */

#endif /* __APP_COLORS_H */

