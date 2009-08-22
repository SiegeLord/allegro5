/*         ______   ___    ___ 
 *        /\  _  \ /\_ \  /\_ \ 
 *        \ \ \L\ \\//\ \ \//\ \      __     __   _ __   ___ 
 *         \ \  __ \ \ \ \  \ \ \   /'__`\ /'_ `\/\`'__\/ __`\
 *          \ \ \/\ \ \_\ \_ \_\ \_/\  __//\ \L\ \ \ \//\ \L\ \
 *           \ \_\ \_\/\____\/\____\ \____\ \____ \ \_\\ \____/
 *            \/_/\/_/\/____/\/____/\/____/\/___L\ \/_/ \/___/
 *                                           /\____/
 *                                           \_/__/
 *
 *      Fixed point math routines and lookup tables.
 *
 *      By Shawn Hargreaves.
 *
 *      See readme.txt for copyright information.
 */


#include <math.h>

#include "allegro5/allegro5.h"



al_fixed _al_fix_cos_tbl[512] =
{
   /* precalculated fixed point (16.16) cosines for a full circle (0-255) */

   65536L,  65531L,  65516L,  65492L,  65457L,  65413L,  65358L,  65294L,
   65220L,  65137L,  65043L,  64940L,  64827L,  64704L,  64571L,  64429L,
   64277L,  64115L,  63944L,  63763L,  63572L,  63372L,  63162L,  62943L,
   62714L,  62476L,  62228L,  61971L,  61705L,  61429L,  61145L,  60851L,
   60547L,  60235L,  59914L,  59583L,  59244L,  58896L,  58538L,  58172L,
   57798L,  57414L,  57022L,  56621L,  56212L,  55794L,  55368L,  54934L,
   54491L,  54040L,  53581L,  53114L,  52639L,  52156L,  51665L,  51166L,
   50660L,  50146L,  49624L,  49095L,  48559L,  48015L,  47464L,  46906L,
   46341L,  45769L,  45190L,  44604L,  44011L,  43412L,  42806L,  42194L,
   41576L,  40951L,  40320L,  39683L,  39040L,  38391L,  37736L,  37076L,
   36410L,  35738L,  35062L,  34380L,  33692L,  33000L,  32303L,  31600L,
   30893L,  30182L,  29466L,  28745L,  28020L,  27291L,  26558L,  25821L,
   25080L,  24335L,  23586L,  22834L,  22078L,  21320L,  20557L,  19792L,
   19024L,  18253L,  17479L,  16703L,  15924L,  15143L,  14359L,  13573L,
   12785L,  11996L,  11204L,  10411L,  9616L,   8820L,   8022L,   7224L,
   6424L,   5623L,   4821L,   4019L,   3216L,   2412L,   1608L,   804L,
   0L,      -804L,   -1608L,  -2412L,  -3216L,  -4019L,  -4821L,  -5623L,
   -6424L,  -7224L,  -8022L,  -8820L,  -9616L,  -10411L, -11204L, -11996L,
   -12785L, -13573L, -14359L, -15143L, -15924L, -16703L, -17479L, -18253L,
   -19024L, -19792L, -20557L, -21320L, -22078L, -22834L, -23586L, -24335L,
   -25080L, -25821L, -26558L, -27291L, -28020L, -28745L, -29466L, -30182L,
   -30893L, -31600L, -32303L, -33000L, -33692L, -34380L, -35062L, -35738L,
   -36410L, -37076L, -37736L, -38391L, -39040L, -39683L, -40320L, -40951L,
   -41576L, -42194L, -42806L, -43412L, -44011L, -44604L, -45190L, -45769L,
   -46341L, -46906L, -47464L, -48015L, -48559L, -49095L, -49624L, -50146L,
   -50660L, -51166L, -51665L, -52156L, -52639L, -53114L, -53581L, -54040L,
   -54491L, -54934L, -55368L, -55794L, -56212L, -56621L, -57022L, -57414L,
   -57798L, -58172L, -58538L, -58896L, -59244L, -59583L, -59914L, -60235L,
   -60547L, -60851L, -61145L, -61429L, -61705L, -61971L, -62228L, -62476L,
   -62714L, -62943L, -63162L, -63372L, -63572L, -63763L, -63944L, -64115L,
   -64277L, -64429L, -64571L, -64704L, -64827L, -64940L, -65043L, -65137L,
   -65220L, -65294L, -65358L, -65413L, -65457L, -65492L, -65516L, -65531L,
   -65536L, -65531L, -65516L, -65492L, -65457L, -65413L, -65358L, -65294L,
   -65220L, -65137L, -65043L, -64940L, -64827L, -64704L, -64571L, -64429L,
   -64277L, -64115L, -63944L, -63763L, -63572L, -63372L, -63162L, -62943L,
   -62714L, -62476L, -62228L, -61971L, -61705L, -61429L, -61145L, -60851L,
   -60547L, -60235L, -59914L, -59583L, -59244L, -58896L, -58538L, -58172L,
   -57798L, -57414L, -57022L, -56621L, -56212L, -55794L, -55368L, -54934L,
   -54491L, -54040L, -53581L, -53114L, -52639L, -52156L, -51665L, -51166L,
   -50660L, -50146L, -49624L, -49095L, -48559L, -48015L, -47464L, -46906L,
   -46341L, -45769L, -45190L, -44604L, -44011L, -43412L, -42806L, -42194L,
   -41576L, -40951L, -40320L, -39683L, -39040L, -38391L, -37736L, -37076L,
   -36410L, -35738L, -35062L, -34380L, -33692L, -33000L, -32303L, -31600L,
   -30893L, -30182L, -29466L, -28745L, -28020L, -27291L, -26558L, -25821L,
   -25080L, -24335L, -23586L, -22834L, -22078L, -21320L, -20557L, -19792L,
   -19024L, -18253L, -17479L, -16703L, -15924L, -15143L, -14359L, -13573L,
   -12785L, -11996L, -11204L, -10411L, -9616L,  -8820L,  -8022L,  -7224L,
   -6424L,  -5623L,  -4821L,  -4019L,  -3216L,  -2412L,  -1608L,  -804L,
   0L,      804L,    1608L,   2412L,   3216L,   4019L,   4821L,   5623L,
   6424L,   7224L,   8022L,   8820L,   9616L,   10411L,  11204L,  11996L,
   12785L,  13573L,  14359L,  15143L,  15924L,  16703L,  17479L,  18253L,
   19024L,  19792L,  20557L,  21320L,  22078L,  22834L,  23586L,  24335L,
   25080L,  25821L,  26558L,  27291L,  28020L,  28745L,  29466L,  30182L,
   30893L,  31600L,  32303L,  33000L,  33692L,  34380L,  35062L,  35738L,
   36410L,  37076L,  37736L,  38391L,  39040L,  39683L,  40320L,  40951L,
   41576L,  42194L,  42806L,  43412L,  44011L,  44604L,  45190L,  45769L,
   46341L,  46906L,  47464L,  48015L,  48559L,  49095L,  49624L,  50146L,
   50660L,  51166L,  51665L,  52156L,  52639L,  53114L,  53581L,  54040L,
   54491L,  54934L,  55368L,  55794L,  56212L,  56621L,  57022L,  57414L,
   57798L,  58172L,  58538L,  58896L,  59244L,  59583L,  59914L,  60235L,
   60547L,  60851L,  61145L,  61429L,  61705L,  61971L,  62228L,  62476L,
   62714L,  62943L,  63162L,  63372L,  63572L,  63763L,  63944L,  64115L,
   64277L,  64429L,  64571L,  64704L,  64827L,  64940L,  65043L,  65137L,
   65220L,  65294L,  65358L,  65413L,  65457L,  65492L,  65516L,  65531L
};



al_fixed _al_fix_tan_tbl[256] =
{
   /* precalculated fixed point (16.16) tangents for a half circle (0-127) */

   0L,      804L,    1609L,   2414L,   3220L,   4026L,   4834L,   5644L,
   6455L,   7268L,   8083L,   8901L,   9721L,   10545L,  11372L,  12202L,
   13036L,  13874L,  14717L,  15564L,  16416L,  17273L,  18136L,  19005L,
   19880L,  20762L,  21650L,  22546L,  23449L,  24360L,  25280L,  26208L,
   27146L,  28093L,  29050L,  30018L,  30996L,  31986L,  32988L,  34002L,
   35030L,  36071L,  37126L,  38196L,  39281L,  40382L,  41500L,  42636L,
   43790L,  44963L,  46156L,  47369L,  48605L,  49863L,  51145L,  52451L,
   53784L,  55144L,  56532L,  57950L,  59398L,  60880L,  62395L,  63947L,
   65536L,  67165L,  68835L,  70548L,  72308L,  74116L,  75974L,  77887L,
   79856L,  81885L,  83977L,  86135L,  88365L,  90670L,  93054L,  95523L,
   98082L,  100736L, 103493L, 106358L, 109340L, 112447L, 115687L, 119071L,
   122609L, 126314L, 130198L, 134276L, 138564L, 143081L, 147847L, 152884L,
   158218L, 163878L, 169896L, 176309L, 183161L, 190499L, 198380L, 206870L,
   216043L, 225990L, 236817L, 248648L, 261634L, 275959L, 291845L, 309568L,
   329472L, 351993L, 377693L, 407305L, 441808L, 482534L, 531352L, 590958L,
   665398L, 761030L, 888450L, 1066730L,1334016L,1779314L,2669641L,5340086L,
   -2147483647L,-5340086L,-2669641L,-1779314L,-1334016L,-1066730L,-888450L,-761030L,
   -665398L,-590958L,-531352L,-482534L,-441808L,-407305L,-377693L,-351993L,
   -329472L,-309568L,-291845L,-275959L,-261634L,-248648L,-236817L,-225990L,
   -216043L,-206870L,-198380L,-190499L,-183161L,-176309L,-169896L,-163878L,
   -158218L,-152884L,-147847L,-143081L,-138564L,-134276L,-130198L,-126314L,
   -122609L,-119071L,-115687L,-112447L,-109340L,-106358L,-103493L,-100736L,
   -98082L, -95523L, -93054L, -90670L, -88365L, -86135L, -83977L, -81885L,
   -79856L, -77887L, -75974L, -74116L, -72308L, -70548L, -68835L, -67165L,
   -65536L, -63947L, -62395L, -60880L, -59398L, -57950L, -56532L, -55144L,
   -53784L, -52451L, -51145L, -49863L, -48605L, -47369L, -46156L, -44963L,
   -43790L, -42636L, -41500L, -40382L, -39281L, -38196L, -37126L, -36071L,
   -35030L, -34002L, -32988L, -31986L, -30996L, -30018L, -29050L, -28093L,
   -27146L, -26208L, -25280L, -24360L, -23449L, -22546L, -21650L, -20762L,
   -19880L, -19005L, -18136L, -17273L, -16416L, -15564L, -14717L, -13874L,
   -13036L, -12202L, -11372L, -10545L, -9721L,  -8901L,  -8083L,  -7268L,
   -6455L,  -5644L,  -4834L,  -4026L,  -3220L,  -2414L,  -1609L,  -804L
};



al_fixed _al_fix_acos_tbl[513] =
{
   /* precalculated fixed point (16.16) inverse cosines (-1 to 1) */

   0x800000L,  0x7C65C7L,  0x7AE75AL,  0x79C19EL,  0x78C9BEL,  0x77EF25L,  0x772953L,  0x76733AL,
   0x75C991L,  0x752A10L,  0x74930CL,  0x740345L,  0x7379C1L,  0x72F5BAL,  0x72768FL,  0x71FBBCL,
   0x7184D3L,  0x711174L,  0x70A152L,  0x703426L,  0x6FC9B5L,  0x6F61C9L,  0x6EFC36L,  0x6E98D1L,
   0x6E3777L,  0x6DD805L,  0x6D7A5EL,  0x6D1E68L,  0x6CC40BL,  0x6C6B2FL,  0x6C13C1L,  0x6BBDAFL,
   0x6B68E6L,  0x6B1558L,  0x6AC2F5L,  0x6A71B1L,  0x6A217EL,  0x69D251L,  0x698420L,  0x6936DFL,
   0x68EA85L,  0x689F0AL,  0x685465L,  0x680A8DL,  0x67C17DL,  0x67792CL,  0x673194L,  0x66EAAFL,
   0x66A476L,  0x665EE5L,  0x6619F5L,  0x65D5A2L,  0x6591E7L,  0x654EBFL,  0x650C26L,  0x64CA18L,
   0x648890L,  0x64478CL,  0x640706L,  0x63C6FCL,  0x63876BL,  0x63484FL,  0x6309A5L,  0x62CB6AL,
   0x628D9CL,  0x625037L,  0x621339L,  0x61D69FL,  0x619A68L,  0x615E90L,  0x612316L,  0x60E7F7L,
   0x60AD31L,  0x6072C3L,  0x6038A9L,  0x5FFEE3L,  0x5FC56EL,  0x5F8C49L,  0x5F5372L,  0x5F1AE7L,
   0x5EE2A7L,  0x5EAAB0L,  0x5E7301L,  0x5E3B98L,  0x5E0473L,  0x5DCD92L,  0x5D96F3L,  0x5D6095L,
   0x5D2A76L,  0x5CF496L,  0x5CBEF2L,  0x5C898BL,  0x5C545EL,  0x5C1F6BL,  0x5BEAB0L,  0x5BB62DL,
   0x5B81E1L,  0x5B4DCAL,  0x5B19E7L,  0x5AE638L,  0x5AB2BCL,  0x5A7F72L,  0x5A4C59L,  0x5A1970L,
   0x59E6B6L,  0x59B42AL,  0x5981CCL,  0x594F9BL,  0x591D96L,  0x58EBBDL,  0x58BA0EL,  0x588889L,
   0x58572DL,  0x5825FAL,  0x57F4EEL,  0x57C40AL,  0x57934DL,  0x5762B5L,  0x573243L,  0x5701F5L,
   0x56D1CCL,  0x56A1C6L,  0x5671E4L,  0x564224L,  0x561285L,  0x55E309L,  0x55B3ADL,  0x558471L,
   0x555555L,  0x552659L,  0x54F77BL,  0x54C8BCL,  0x549A1BL,  0x546B98L,  0x543D31L,  0x540EE7L,
   0x53E0B9L,  0x53B2A7L,  0x5384B0L,  0x5356D4L,  0x532912L,  0x52FB6BL,  0x52CDDDL,  0x52A068L,
   0x52730CL,  0x5245C9L,  0x52189EL,  0x51EB8BL,  0x51BE8FL,  0x5191AAL,  0x5164DCL,  0x513825L,
   0x510B83L,  0x50DEF7L,  0x50B280L,  0x50861FL,  0x5059D2L,  0x502D99L,  0x500175L,  0x4FD564L,
   0x4FA967L,  0x4F7D7DL,  0x4F51A6L,  0x4F25E2L,  0x4EFA30L,  0x4ECE90L,  0x4EA301L,  0x4E7784L,
   0x4E4C19L,  0x4E20BEL,  0x4DF574L,  0x4DCA3AL,  0x4D9F10L,  0x4D73F6L,  0x4D48ECL,  0x4D1DF1L,
   0x4CF305L,  0x4CC829L,  0x4C9D5AL,  0x4C729AL,  0x4C47E9L,  0x4C1D45L,  0x4BF2AEL,  0x4BC826L,
   0x4B9DAAL,  0x4B733BL,  0x4B48D9L,  0x4B1E84L,  0x4AF43BL,  0x4AC9FEL,  0x4A9FCDL,  0x4A75A7L,
   0x4A4B8DL,  0x4A217EL,  0x49F77AL,  0x49CD81L,  0x49A393L,  0x4979AFL,  0x494FD5L,  0x492605L,
   0x48FC3FL,  0x48D282L,  0x48A8CFL,  0x487F25L,  0x485584L,  0x482BECL,  0x48025DL,  0x47D8D6L,
   0x47AF57L,  0x4785E0L,  0x475C72L,  0x47330AL,  0x4709ABL,  0x46E052L,  0x46B701L,  0x468DB7L,
   0x466474L,  0x463B37L,  0x461201L,  0x45E8D0L,  0x45BFA6L,  0x459682L,  0x456D64L,  0x45444BL,
   0x451B37L,  0x44F229L,  0x44C920L,  0x44A01CL,  0x44771CL,  0x444E21L,  0x44252AL,  0x43FC38L,
   0x43D349L,  0x43AA5FL,  0x438178L,  0x435894L,  0x432FB4L,  0x4306D8L,  0x42DDFEL,  0x42B527L,
   0x428C53L,  0x426381L,  0x423AB2L,  0x4211E5L,  0x41E91AL,  0x41C051L,  0x41978AL,  0x416EC5L,
   0x414601L,  0x411D3EL,  0x40F47CL,  0x40CBBBL,  0x40A2FBL,  0x407A3CL,  0x40517DL,  0x4028BEL,
   0x400000L,  0x3FD742L,  0x3FAE83L,  0x3F85C4L,  0x3F5D05L,  0x3F3445L,  0x3F0B84L,  0x3EE2C2L,
   0x3EB9FFL,  0x3E913BL,  0x3E6876L,  0x3E3FAFL,  0x3E16E6L,  0x3DEE1BL,  0x3DC54EL,  0x3D9C7FL,
   0x3D73ADL,  0x3D4AD9L,  0x3D2202L,  0x3CF928L,  0x3CD04CL,  0x3CA76CL,  0x3C7E88L,  0x3C55A1L,
   0x3C2CB7L,  0x3C03C8L,  0x3BDAD6L,  0x3BB1DFL,  0x3B88E4L,  0x3B5FE4L,  0x3B36E0L,  0x3B0DD7L,
   0x3AE4C9L,  0x3ABBB5L,  0x3A929CL,  0x3A697EL,  0x3A405AL,  0x3A1730L,  0x39EDFFL,  0x39C4C9L,
   0x399B8CL,  0x397249L,  0x3948FFL,  0x391FAEL,  0x38F655L,  0x38CCF6L,  0x38A38EL,  0x387A20L,
   0x3850A9L,  0x38272AL,  0x37FDA3L,  0x37D414L,  0x37AA7CL,  0x3780DBL,  0x375731L,  0x372D7EL,
   0x3703C1L,  0x36D9FBL,  0x36B02BL,  0x368651L,  0x365C6DL,  0x36327FL,  0x360886L,  0x35DE82L,
   0x35B473L,  0x358A59L,  0x356033L,  0x353602L,  0x350BC5L,  0x34E17CL,  0x34B727L,  0x348CC5L,
   0x346256L,  0x3437DAL,  0x340D52L,  0x33E2BBL,  0x33B817L,  0x338D66L,  0x3362A6L,  0x3337D7L,
   0x330CFBL,  0x32E20FL,  0x32B714L,  0x328C0AL,  0x3260F0L,  0x3235C6L,  0x320A8CL,  0x31DF42L,
   0x31B3E7L,  0x31887CL,  0x315CFFL,  0x313170L,  0x3105D0L,  0x30DA1EL,  0x30AE5AL,  0x308283L,
   0x305699L,  0x302A9CL,  0x2FFE8BL,  0x2FD267L,  0x2FA62EL,  0x2F79E1L,  0x2F4D80L,  0x2F2109L,
   0x2EF47DL,  0x2EC7DBL,  0x2E9B24L,  0x2E6E56L,  0x2E4171L,  0x2E1475L,  0x2DE762L,  0x2DBA37L,
   0x2D8CF4L,  0x2D5F98L,  0x2D3223L,  0x2D0495L,  0x2CD6EEL,  0x2CA92CL,  0x2C7B50L,  0x2C4D59L,
   0x2C1F47L,  0x2BF119L,  0x2BC2CFL,  0x2B9468L,  0x2B65E5L,  0x2B3744L,  0x2B0885L,  0x2AD9A7L,
   0x2AAAABL,  0x2A7B8FL,  0x2A4C53L,  0x2A1CF7L,  0x29ED7BL,  0x29BDDCL,  0x298E1CL,  0x295E3AL,
   0x292E34L,  0x28FE0BL,  0x28CDBDL,  0x289D4BL,  0x286CB3L,  0x283BF6L,  0x280B12L,  0x27DA06L,
   0x27A8D3L,  0x277777L,  0x2745F2L,  0x271443L,  0x26E26AL,  0x26B065L,  0x267E34L,  0x264BD6L,
   0x26194AL,  0x25E690L,  0x25B3A7L,  0x25808EL,  0x254D44L,  0x2519C8L,  0x24E619L,  0x24B236L,
   0x247E1FL,  0x2449D3L,  0x241550L,  0x23E095L,  0x23ABA2L,  0x237675L,  0x23410EL,  0x230B6AL,
   0x22D58AL,  0x229F6BL,  0x22690DL,  0x22326EL,  0x21FB8DL,  0x21C468L,  0x218CFFL,  0x215550L,
   0x211D59L,  0x20E519L,  0x20AC8EL,  0x2073B7L,  0x203A92L,  0x20011DL,  0x1FC757L,  0x1F8D3DL,
   0x1F52CFL,  0x1F1809L,  0x1EDCEAL,  0x1EA170L,  0x1E6598L,  0x1E2961L,  0x1DECC7L,  0x1DAFC9L,
   0x1D7264L,  0x1D3496L,  0x1CF65BL,  0x1CB7B1L,  0x1C7895L,  0x1C3904L,  0x1BF8FAL,  0x1BB874L,
   0x1B7770L,  0x1B35E8L,  0x1AF3DAL,  0x1AB141L,  0x1A6E19L,  0x1A2A5EL,  0x19E60BL,  0x19A11BL,
   0x195B8AL,  0x191551L,  0x18CE6CL,  0x1886D4L,  0x183E83L,  0x17F573L,  0x17AB9BL,  0x1760F6L,
   0x17157BL,  0x16C921L,  0x167BE0L,  0x162DAFL,  0x15DE82L,  0x158E4FL,  0x153D0BL,  0x14EAA8L,
   0x14971AL,  0x144251L,  0x13EC3FL,  0x1394D1L,  0x133BF5L,  0x12E198L,  0x1285A2L,  0x1227FBL,
   0x11C889L,  0x11672FL,  0x1103CAL,  0x109E37L,  0x10364BL,  0xFCBDAL,   0xF5EAEL,   0xEEE8CL,
   0xE7B2DL,   0xE0444L,   0xD8971L,   0xD0A46L,   0xC863FL,   0xBFCBBL,   0xB6CF4L,   0xAD5F0L,
   0xA366FL,   0x98CC6L,   0x8D6ADL,   0x810DBL,   0x73642L,   0x63E62L,   0x518A6L,   0x39A39L,
   0x0L
};



/* Function: al_fixatan
 *  Fixed point inverse tangent. Does a binary search on the tan table.
 */
al_fixed al_fixatan(al_fixed x)
{
   int a, b, c;            /* for binary search */
   al_fixed d;                /* difference value for search */

   if (x >= 0) {           /* search the first part of tan table */
      a = 0;
      b = 127;
   }
   else {                  /* search the second half instead */
      a = 128;
      b = 255;
   } 

   do {
      c = (a + b) >> 1;
      d = x - _al_fix_tan_tbl[c];

      if (d > 0)
	 a = c + 1;
      else
	 if (d < 0)
	    b = c - 1;

   } while ((a <= b) && (d));

   if (x >= 0)
      return ((long)c) << 15;

   return (-0x00800000L + (((long)c) << 15));
}



/* Function: al_fixatan2
 *  Like the libc atan2, but for fixed point numbers.
 */
al_fixed al_fixatan2(al_fixed y, al_fixed x)
{
   al_fixed r;

   if (x==0) {
      if (y==0) {
	 al_set_errno(EDOM);
	 return 0L;
      }
      else
	 return ((y < 0) ? -0x00400000L : 0x00400000L);
   } 

   al_set_errno(0);
   r = al_fixdiv(y, x);

   if (al_get_errno()) {
      al_set_errno(0);
      return ((y < 0) ? -0x00400000L : 0x00400000L);
   }

   r = al_fixatan(r);

   if (x >= 0)
      return r;

   if (y >= 0)
      return 0x00800000L + r;

   return r - 0x00800000L;
}



/* Enum: al_fixtorad_r
 *  Ratios for converting between radians and fixed point angles.
 *  2pi/256
 */
const al_fixed al_fixtorad_r = (al_fixed)1608;



/* Enum: al_radtofix_r
 *  Ratios for converting between radians and fixed point angles.
 *  256/2pi
 */
const al_fixed al_radtofix_r = (al_fixed)2670177;



/* Function: al_fixsqrt
 *  Fixed point square root routine for non-i386.
 */
al_fixed al_fixsqrt(al_fixed x)
{
   if (x > 0)
      return al_ftofix(sqrt(al_fixtof(x)));

   if (x < 0)
      al_set_errno(EDOM);

   return 0;
}



/* Function: al_fixhypot
 *  Fixed point sqrt (x*x+y*y) for non-i386.
 */
al_fixed al_fixhypot(al_fixed x, al_fixed y)
{
   return al_ftofix(hypot(al_fixtof(x), al_fixtof(y)));
}



/* These prototypes exist for documentation only. */

/* Function: al_itofix
 */
al_fixed al_itofix(int x);

/* Function: al_fixtoi
 */
int al_fixtoi(al_fixed x);

/* Function: al_fixfloor
 */
int al_fixfloor(al_fixed x);

/* Function: al_fixceil
 */
int al_fixceil(al_fixed x);

/* Function: al_ftofix
 */
al_fixed al_ftofix(double x);

/* Function: al_fixtof
 */
double al_fixtof(al_fixed x);

/* Function: al_fixadd
 */
al_fixed al_fixadd(al_fixed x, al_fixed y);

/* Function: al_fixsub
 */
al_fixed al_fixsub(al_fixed x, al_fixed y);

/* Function: al_fixmul
 */
al_fixed al_fixmul(al_fixed x, al_fixed y);

/* Function: al_fixdiv
 */
al_fixed al_fixdiv(al_fixed x, al_fixed y);

/* Function: al_fixcos
 */
al_fixed al_fixcos(al_fixed x);

/* Function: al_fixsin
 */
al_fixed al_fixsin(al_fixed x);

/* Function: al_fixtan
 */
al_fixed al_fixtan(al_fixed x);

/* Function: al_fixacos
 */
al_fixed al_fixacos(al_fixed x);

/* Function: al_fixasin
 */
al_fixed al_fixasin(al_fixed x);

