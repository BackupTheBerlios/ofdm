/* 00  00    32767    +0i */
/* 01  01    23169+23169i */
/* 02  03        0+32767i */
/* 03  02   -23169+23169i */
/* 04  06   -32767    +0i */
/* 05  07   -23169-23169i */
/* 06  05        0-32767i */
/* 07  04    23169-23169i */
/* train sequence: min max offdiag a*a^H: 0.343233 */

const cplxshort_t psk_symmapping[8] = {
	{ 32767, 0 },
	{ 23169, 23169 },
	{ -23169, 23169 },
	{ 0, 32767 },
	{ 23169, -23169 },
	{ 0, -32767 },
	{ -32767, 0 },
	{ -23169, -23169 }
};

/*
  trsym = [ -32767+0i 23169-23169i -32767+0i -23169-23169i 0+32767i 0+32767i 23169+23169i 0-32767i -23169+23169i 23169+23169i -23169+23169i -23169+23169i -23169-23169i 0-32767i 0+32767i -23169-23169i ];
  trsymcc = [ 15.9984+0i -0.414208+0.414208i -0.414208-1.58554i -6.62794e-05+1.99988i 2.41395-1.58554i 1.41408-3.41376i 2.24263+2.82816i -4.41376-2.41402i 2.24236+0i -4.41376+2.41402i 2.24263-2.82816i 1.41408+3.41376i 2.41395+1.58554i -6.62794e-05-1.99988i -0.414208+1.58554i -0.414208-0.414208i ];
a = [ -0.999969 0.707062-0.707062i -0.999969 ; ...
     0.707062-0.707062i -0.999969 -0.707062-0.707062i ; ...
     -0.999969 -0.707062-0.707062i 0+0.999969i ; ...
     -0.707062-0.707062i 0+0.999969i 0+0.999969i ; ...
     0+0.999969i 0+0.999969i 0.707062+0.707062i ; ...
     0+0.999969i 0.707062+0.707062i 0-0.999969i ; ...
     0.707062+0.707062i 0-0.999969i -0.707062+0.707062i ; ...
     0-0.999969i -0.707062+0.707062i 0.707062+0.707062i ; ...
     -0.707062+0.707062i 0.707062+0.707062i -0.707062+0.707062i ; ...
     0.707062+0.707062i -0.707062+0.707062i -0.707062+0.707062i ; ...
     -0.707062+0.707062i -0.707062+0.707062i -0.707062-0.707062i ; ...
     -0.707062+0.707062i -0.707062-0.707062i 0-0.999969i ; ...
     -0.707062-0.707062i 0-0.999969i 0+0.999969i ; ...
     0-0.999969i 0+0.999969i -0.707062-0.707062i ];
ahainvah = [ -0.0737285+0.00084296i 0.0453474+0.0484783i -0.0725354-0.00203612i -0.0463163+0.0463167i 0.00521937-0.0738541i 0.00084296-0.0696563i 0.0486566-0.051536i 0.0030054+0.0686875i -0.0475093-0.0538758i 0.0506928-0.0568809i -0.0538758-0.0538758i -0.0560378-0.0484786i -0.0505664+0.0508183i -0.000969272+0.0693525i ; ...
     0.0461909+0.0506925i -0.0716924+0.000177842i -0.0508183+0.0505667i 0.00203613-0.0708491i 0.000969046-0.0740321i 0.0464424-0.0506925i -3.14557e-10+0.0655042i -0.0454734-0.0485306i 0.0475095-0.0538755i -0.0506925-0.0568807i -0.0568807-0.0506925i -0.0559117+0.0528544i 0.00203613+0.0725357i -0.000969046-0.0693527i ; ...
     -0.0707234+0.00234006i -0.049849+0.0527284i -0.00221396-0.0663472i -0.00203589-0.0708489i 0.0463163-0.0550684i -0.00216222+0.0664733i -0.0486566-0.051536i 0.0498496-0.0486564i -0.0536979-0.0536975i -0.0538758-0.0538758i -0.0568809+0.0506928i -0.00318324+0.0748757i 0.00221421-0.0663473i -0.0505664+0.0505668i ];
 */


static const unsigned char trainsyms[16] = {
	6, 4, 6, 7, 3, 3, 1, 5, 2, 1, 2, 2, 7, 5, 3, 7
};

static const cplxshort_t traincorrrotated[16] = {

	{ -32767, 0 }, { -23169, 23168 }, { 32767, 0 }, { 23168, 23169 }, 
	{ 0, -32767 }, { 32767, 0 }, { -23169, 23168 }, { 32767, 0 }, 
	{ -23168, -23169 }, { 23168, 23169 }, { 23168, 23169 }, { -23169, 23168 }, 
	{ -23169, 23168 }, { -32767, 0 }, { 0, 32767 }, { 23168, 23169 }
};

static const cplxshort_t trainmatrotated[42] = {
	{ 4831, -55 }, { 3177, -2971 }, { -4753, -133 }, { -3035, -3035 }, { -342, 4840 }, { -4564, -55 }, { 3188, -3377 }, { -4501, 196 },
	{ 3113, 3530 }, { -3727, -3322 }, { -3530, -3530 }, { 3177, -3672 }, { 3313, -3330 }, { 4545, 63 }, 
	{ -3027, -3322 }, { 11, 4698 }, { -3330, 3313 }, { 4643, 133 }, { -63, 4851 }, { -3322, -3043 }, { 0, 4292 }, { 3180, -2980 },
	{ -3113, 3530 }, { -3727, 3322 }, { -3727, -3322 }, { -3463, -3664 }, { -133, -4753 }, { -4545, 63 }, 
	{ 4634, -153 }, { 3455, 3266 }, { -145, -4348 }, { 4643, -133 }, { -3035, 3608 }, { 4356, 141 }, { -3188, -3377 }, { 3188, 3266 },
	{ 3519, 3519 }, { -3530, 3530 }, { -3727, 3322 }, { -4907, -208 }, { -145, 4348 }, { 3313, 3313 }
};

#define MLSEROOTNODE  0x1f
#define MLSETOORNODE  0x34

/*
  txfilt = [ -7.89185e-19 ; -0.000169639 ; -0.000356223 ; -0.000549436 ;
             -0.000737741 ; -0.000909138 ; -0.001052 ; -0.00115595 ;
             -0.00121261 ; -0.00121637 ; -0.00116487 ; -0.00105933 ;
             -0.000904593 ; -0.000708972 ; -0.000483741 ; -0.000242448 ;
             1.03873e-18 ; 0.000228389 ; 0.000428328 ; 0.000587364 ;
             0.000696013 ; 0.000748609 ; 0.000743905 ; 0.000685371 ;
             0.000581136 ; 0.000443577 ; 0.000288539 ; 0.000134254 ;
             -4.59389e-19 ; -9.5412e-05 ; -0.000135207 ; -0.00010617 ;
             5.46534e-19 ; 0.000185619 ; 0.000446326 ; 0.00077085 ;
             0.00114129 ; 0.0015339 ; 0.00192042 ; 0.00226978 ;
             0.00255019 ; 0.00273144 ; 0.00278726 ; 0.00269755 ;
             0.00245035 ; 0.00204336 ; 0.00148485 ; 0.000793942 ;
             -2.5987e-18 ; -0.000858639 ; -0.00173676 ; -0.00258502 ;
             -0.00335312 ; -0.00399341 ; -0.0044645 ; -0.00473465 ;
             -0.0047848 ; -0.00461084 ; -0.00422493 ; -0.00365583 ;
             -0.00294782 ; -0.00215855 ; -0.00135555 ; -0.000611731 ;
             1.30364e-18 ; 0.000412633 ; 0.000571146 ; 0.000438343 ;
             -1.37817e-18 ; -0.000731099 ; -0.00171266 ; -0.00287225 ;
             -0.00410874 ; -0.00529605 ; -0.00628909 ; -0.00693185 ;
             -0.00706721 ; -0.00654822 ; -0.00525007 ; -0.00308225 ;
             6.62517e-18 ; 0.00398561 ; 0.00879986 ; 0.0142994 ;
             0.0202711 ; 0.0264345 ; 0.032449 ; 0.0379254 ;
             0.0424413 ; 0.045561 ; 0.0468577 ; 0.0459391 ;
             0.0424728 ; 0.0362128 ; 0.027024 ; 0.0149038 ;
             -2.02153e-17 ; -0.0173775 ; -0.0367505 ; -0.0574747 ;
             -0.0787489 ; -0.0996327 ; -0.119072 ; -0.135932 ;
             -0.149035 ; -0.157207 ; -0.159322 ; -0.154354 ;
             -0.141421 ; -0.119838 ; -0.0891489 ; -0.0491676 ;
             3.346e-17 ; 0.0579397 ; 0.123925 ; 0.196924 ;
             0.275621 ; 0.358446 ; 0.443618 ; 0.529196 ;
             0.613138 ; 0.693369 ; 0.767845 ; 0.834623 ;
             0.891929 ; 0.938215 ; 0.97222 ; 0.993006 ;
             1 ; 0.993006 ; 0.97222 ; 0.938215 ;
             0.891929 ; 0.834623 ; 0.767845 ; 0.693369 ;
             0.613138 ; 0.529196 ; 0.443618 ; 0.358446 ;
             0.275621 ; 0.196924 ; 0.123925 ; 0.0579397 ;
             3.346e-17 ; -0.0491676 ; -0.0891489 ; -0.119838 ;
             -0.141421 ; -0.154354 ; -0.159322 ; -0.157207 ;
             -0.149035 ; -0.135932 ; -0.119072 ; -0.0996327 ;
             -0.0787489 ; -0.0574747 ; -0.0367505 ; -0.0173775 ;
             -2.02153e-17 ; 0.0149038 ; 0.027024 ; 0.0362128 ;
             0.0424728 ; 0.0459391 ; 0.0468577 ; 0.045561 ;
             0.0424413 ; 0.0379254 ; 0.032449 ; 0.0264345 ;
             0.0202711 ; 0.0142994 ; 0.00879986 ; 0.00398561 ;
             6.62517e-18 ; -0.00308225 ; -0.00525007 ; -0.00654822 ;
             -0.00706721 ; -0.00693185 ; -0.00628909 ; -0.00529605 ;
             -0.00410874 ; -0.00287225 ; -0.00171266 ; -0.000731099 ;
             -1.37817e-18 ; 0.000438343 ; 0.000571146 ; 0.000412633 ;
             1.30364e-18 ; -0.000611731 ; -0.00135555 ; -0.00215855 ;
             -0.00294782 ; -0.00365583 ; -0.00422493 ; -0.00461084 ;
             -0.0047848 ; -0.00473465 ; -0.0044645 ; -0.00399341 ;
             -0.00335312 ; -0.00258502 ; -0.00173676 ; -0.000858639 ;
             -2.5987e-18 ; 0.000793942 ; 0.00148485 ; 0.00204336 ;
             0.00245035 ; 0.00269755 ; 0.00278726 ; 0.00273144 ;
             0.00255019 ; 0.00226978 ; 0.00192042 ; 0.0015339 ;
             0.00114129 ; 0.00077085 ; 0.000446326 ; 0.000185619 ;
             5.46534e-19 ; -0.00010617 ; -0.000135207 ; -9.5412e-05 ;
             -4.59389e-19 ; 0.000134254 ; 0.000288539 ; 0.000443577 ;
             0.000581136 ; 0.000685371 ; 0.000743905 ; 0.000748609 ;
             0.000696013 ; 0.000587364 ; 0.000428328 ; 0.000228389 ;
             1.03873e-18 ; -0.000242448 ; -0.000483741 ; -0.000708972 ;
             -0.000904593 ; -0.00105933 ; -0.00116487 ; -0.00121637 ;
             -0.00121261 ; -0.00115595 ; -0.001052 ; -0.000909138 ;
             -0.000737741 ; -0.000549436 ; -0.000356223 ; -0.000169639 ; ];
  abssum = 1.6357;
  semilogy((0:255)/256,abs(fft(txfilt)))
 */

static const int txfilter[16][16] = {
	{ 0, 0, 0, 0, 0, 0, 0, 0, 20032, 0, 0, 0, 0, 0, 0, 0 },
	{ -3, 4, 3, -17, 8, 79, -348, 1160, 19892, -984, 298, -61, -12, 15, -2, -4 },
	{ -7, 8, 8, -34, 11, 176, -736, 2482, 19475, -1785, 541, -105, -27, 29, -2, -9 },
	{ -11, 11, 15, -51, 8, 286, -1151, 3944, 18794, -2400, 725, -131, -43, 40, -1, -14 },
	{ -14, 13, 22, -67, 0, 406, -1577, 5521, 17867, -2833, 850, -141, -59, 49, 0, -18 },
	{ -18, 14, 30, -79, -14, 529, -1995, 7180, 16719, -3092, 920, -138, -73, 54, 2, -21 },
	{ -21, 14, 38, -89, -34, 650, -2385, 8886, 15381, -3191, 938, -125, -84, 55, 5, -23 },
	{ -23, 13, 45, -94, -57, 759, -2723, 10601, 13889, -3149, 912, -106, -92, 54, 8, -24 },
	{ -24, 11, 51, -95, -82, 850, -2985, 12282, 12282, -2985, 850, -82, -95, 51, 11, -24 },
	{ -24, 8, 54, -92, -106, 912, -3149, 13889, 10601, -2723, 759, -57, -94, 45, 13, -23 },
	{ -23, 5, 55, -84, -125, 938, -3191, 15381, 8886, -2385, 650, -34, -89, 38, 14, -21 },
	{ -21, 2, 54, -73, -138, 920, -3092, 16719, 7180, -1995, 529, -14, -79, 30, 14, -18 },
	{ -18, 0, 49, -59, -141, 850, -2833, 17867, 5521, -1577, 406, 0, -67, 22, 13, -14 },
	{ -14, -1, 40, -43, -131, 725, -2400, 18794, 3944, -1151, 286, 8, -51, 15, 11, -11 },
	{ -9, -2, 29, -27, -105, 541, -1785, 19475, 2482, -736, 176, 11, -34, 8, 8, -7 },
	{ -4, -2, 15, -12, -61, 298, -984, 19892, 1160, -348, 79, 8, -17, 3, 4, -3 }
};

/*
  rxfilt = [ 0.00534565 ; 0.00644843 ; 0.00757986 ; 0.00875248 ;
             0.00996825 ; 0.0112164 ; 0.012472 ; 0.0136948 ;
             0.0148292 ; 0.0158048 ; 0.0165376 ; 0.0169325 ;
             0.016886 ; 0.0162902 ; 0.0150367 ; 0.013022 ;
             0.0101524 ; 0.00634969 ; 0.00155649 ; -0.00425822 ;
             -0.0110942 ; -0.0189153 ; -0.0276459 ; -0.0371682 ;
             -0.04732 ; -0.0578947 ; -0.0686416 ; -0.0792683 ;
             -0.0894442 ; -0.0988052 ; -0.10696 ; -0.113498 ;
             -0.117998 ; -0.120035 ; -0.119195 ; -0.115082 ;
             -0.10733 ; -0.0956153 ; -0.0796631 ; -0.0592614 ;
             -0.0342674 ; -0.0046161 ; 0.0296741 ; 0.0684964 ;
             0.111653 ; 0.158856 ; 0.209725 ; 0.263794 ;
             0.320517 ; 0.379273 ; 0.439376 ; 0.500089 ;
             0.560635 ; 0.620208 ; 0.677993 ; 0.733176 ;
             0.784965 ; 0.832601 ; 0.875376 ; 0.912645 ;
             0.943842 ; 0.968488 ; 0.986203 ; 0.996714 ;
             0.999859 ; 0.995592 ; 0.983984 ; 0.965218 ;
             0.93959 ; 0.907502 ; 0.869449 ; 0.826014 ;
             0.777855 ; 0.72569 ; 0.670283 ; 0.612429 ;
             0.552941 ; 0.492629 ; 0.432291 ; 0.372694 ;
             0.314561 ; 0.258562 ; 0.205298 ; 0.155298 ;
             0.109006 ; 0.0667808 ; 0.0288905 ; -0.00448781 ;
             -0.0332666 ; -0.0574451 ; -0.0771043 ; -0.0924002 ;
             -0.103556 ; -0.110854 ; -0.114624 ; -0.115233 ;
             -0.113078 ; -0.108569 ; -0.102125 ; -0.0941583 ;
             -0.0850708 ; -0.0752412 ; -0.0650205 ; -0.0547253 ;
             -0.0446337 ; -0.0349818 ; -0.0259622 ; -0.0177238 ;
             -0.0103722 ; -0.00397234 ; 0.00144888 ; 0.00589848 ;
             0.00941285 ; 0.0120526 ; 0.0138974 ; 0.01504 ;
             0.0155817 ; 0.0156264 ; 0.0152766 ; 0.0146288 ;
             0.0137704 ; 0.0127769 ; 0.0117102 ; 0.0106169 ;
             0.00952902 ; 0.0084636 ; 0.00742438 ; 0.00640338 ; ];
  abssum = 3.6765;
  semilogy((0:127)/128,abs(fft(rxfilt)))
 */

static const int rxfilter_re[8][16] = {
	{ 125, -138, 642, 296, -906, 3776, -3175, -17574, -7569, 3957, -77, -301, 836, -84, 63, 112 },
	{ 141, -126, 490, 553, -1097, 3296, -1184, -16822, -9831, 3632, 512, -593, 895, -22, 26, 126 },
	{ 148, -97, 336, 753, -1144, 2666, 542, -15606, -11986, 2958, 1188, -844, 900, 66, -13, 136 },
	{ 147, -58, 193, 885, -1062, 1952, 1949, -13988, -13925, 1922, 1905, -1025, 842, 181, -54, 141 },
	{ 141, -14, 71, 948, -877, 1219, 3003, -12053, -15553, 535, 2606, -1106, 717, 315, -90, 140 },
	{ 129, 28, -24, 945, -616, 526, 3691, -9899, -16784, -1171, 3227, -1062, 528, 460, -116, 132 },
	{ 113, 68, -90, 885, -313, -79, 4027, -7630, -17554, -3143, 3701, -878, 283, 604, -127, 117 },
	{ 95, 101, -127, 779, 0, -564, 4039, -5354, -17822, -5306, 3964, -547, 0, 735, -118, 93 }
};

static const int rxfilter_im[8][16] = {
	{ -209, -186, -160, -2001, -543, -2801, -12677, -2606, 12629, 5336, 19, 2031, 501, 62, 252, 16 },
	{ -171, -236, -48, -1824, -900, -1762, -12027, -5103, 11980, 6795, -50, 1955, 735, 12, 270, 38 },
	{ -134, -273, 16, -1592, -1262, -953, -11042, -7381, 10863, 8267, 58, 1786, 993, -23, 278, 64 },
	{ -98, -295, 38, -1325, -1590, -388, -9801, -9347, 9304, 9666, 379, 1534, 1260, -36, 272, 94 },
	{ -66, -301, 25, -1046, -1854, -59, -8393, -10925, 7356, 10903, 932, 1220, 1517, -15, 252, 127 },
	{ -39, -293, -13, -776, -2033, 51, -6907, -12061, 5091, 11890, 1725, 871, 1742, 45, 218, 161 },
	{ -16, -273, -67, -530, -2116, -19, -5430, -12729, 2603, 12548, 2745, 526, 1914, 151, 172, 195 },
	{ 0, -244, -127, -322, -2103, -233, -4039, -12927, 0, 12810, 3964, 226, 2015, 304, 118, 226 }
};

static const int16_t rxfilter_re_16[8][16] = {
	{ 125, -138, 642, 296, -906, 3776, -3175, -17574, -7569, 3957, -77, -301, 836, -84, 63, 112 },
	{ 141, -126, 490, 553, -1097, 3296, -1184, -16822, -9831, 3632, 512, -593, 895, -22, 26, 126 },
	{ 148, -97, 336, 753, -1144, 2666, 542, -15606, -11986, 2958, 1188, -844, 900, 66, -13, 136 },
	{ 147, -58, 193, 885, -1062, 1952, 1949, -13988, -13925, 1922, 1905, -1025, 842, 181, -54, 141 },
	{ 141, -14, 71, 948, -877, 1219, 3003, -12053, -15553, 535, 2606, -1106, 717, 315, -90, 140 },
	{ 129, 28, -24, 945, -616, 526, 3691, -9899, -16784, -1171, 3227, -1062, 528, 460, -116, 132 },
	{ 113, 68, -90, 885, -313, -79, 4027, -7630, -17554, -3143, 3701, -878, 283, 604, -127, 117 },
	{ 95, 101, -127, 779, 0, -564, 4039, -5354, -17822, -5306, 3964, -547, 0, 735, -118, 93 }
};

static const int16_t rxfilter_im_16[8][16] = {
	{ -209, -186, -160, -2001, -543, -2801, -12677, -2606, 12629, 5336, 19, 2031, 501, 62, 252, 16 },
	{ -171, -236, -48, -1824, -900, -1762, -12027, -5103, 11980, 6795, -50, 1955, 735, 12, 270, 38 },
	{ -134, -273, 16, -1592, -1262, -953, -11042, -7381, 10863, 8267, 58, 1786, 993, -23, 278, 64 },
	{ -98, -295, 38, -1325, -1590, -388, -9801, -9347, 9304, 9666, 379, 1534, 1260, -36, 272, 94 },
	{ -66, -301, 25, -1046, -1854, -59, -8393, -10925, 7356, 10903, 932, 1220, 1517, -15, 252, 127 },
	{ -39, -293, -13, -776, -2033, 51, -6907, -12061, 5091, 11890, 1725, 871, 1742, 45, 218, 161 },
	{ -16, -273, -67, -530, -2116, -19, -5430, -12729, 2603, 12548, 2745, 526, 1914, 151, 172, 195 },
	{ 0, -244, -127, -322, -2103, -233, -4039, -12927, 0, 12810, 3964, 226, 2015, 304, 118, 226 }
};

static const short costab[512] = {
	 32767,  32764,  32757,  32744,  32727,  32705,  32678,  32646, 
	 32609,  32567,  32520,  32468,  32412,  32350,  32284,  32213, 
	 32137,  32056,  31970,  31880,  31785,  31684,  31580,  31470, 
	 31356,  31236,  31113,  30984,  30851,  30713,  30571,  30424, 
	 30272,  30116,  29955,  29790,  29621,  29446,  29268,  29085, 
	 28897,  28706,  28510,  28309,  28105,  27896,  27683,  27466, 
	 27244,  27019,  26789,  26556,  26318,  26077,  25831,  25582, 
	 25329,  25072,  24811,  24546,  24278,  24006,  23731,  23452, 
	 23169,  22883,  22594,  22301,  22004,  21705,  21402,  21096, 
	 20787,  20474,  20159,  19840,  19519,  19194,  18867,  18537, 
	 18204,  17868,  17530,  17189,  16845,  16499,  16150,  15799, 
	 15446,  15090,  14732,  14372,  14009,  13645,  13278,  12909, 
	 12539,  12166,  11792,  11416,  11038,  10659,  10278,   9895, 
	  9511,   9126,   8739,   8351,   7961,   7571,   7179,   6786, 
	  6392,   5997,   5601,   5205,   4807,   4409,   4011,   3611, 
	  3211,   2811,   2410,   2009,   1607,   1206,    804,    402, 
	     0,   -402,   -804,  -1206,  -1607,  -2009,  -2410,  -2811, 
	 -3211,  -3611,  -4011,  -4409,  -4807,  -5205,  -5601,  -5997, 
	 -6392,  -6786,  -7179,  -7571,  -7961,  -8351,  -8739,  -9126, 
	 -9511,  -9895, -10278, -10659, -11038, -11416, -11792, -12166, 
	-12539, -12909, -13278, -13645, -14009, -14372, -14732, -15090, 
	-15446, -15799, -16150, -16499, -16845, -17189, -17530, -17868, 
	-18204, -18537, -18867, -19194, -19519, -19840, -20159, -20474, 
	-20787, -21096, -21402, -21705, -22004, -22301, -22594, -22883, 
	-23169, -23452, -23731, -24006, -24278, -24546, -24811, -25072, 
	-25329, -25582, -25831, -26077, -26318, -26556, -26789, -27019, 
	-27244, -27466, -27683, -27896, -28105, -28309, -28510, -28706, 
	-28897, -29085, -29268, -29446, -29621, -29790, -29955, -30116, 
	-30272, -30424, -30571, -30713, -30851, -30984, -31113, -31236, 
	-31356, -31470, -31580, -31684, -31785, -31880, -31970, -32056, 
	-32137, -32213, -32284, -32350, -32412, -32468, -32520, -32567, 
	-32609, -32646, -32678, -32705, -32727, -32744, -32757, -32764, 
	-32767, -32764, -32757, -32744, -32727, -32705, -32678, -32646, 
	-32609, -32567, -32520, -32468, -32412, -32350, -32284, -32213, 
	-32137, -32056, -31970, -31880, -31785, -31684, -31580, -31470, 
	-31356, -31236, -31113, -30984, -30851, -30713, -30571, -30424, 
	-30272, -30116, -29955, -29790, -29621, -29446, -29268, -29085, 
	-28897, -28706, -28510, -28309, -28105, -27896, -27683, -27466, 
	-27244, -27019, -26789, -26556, -26318, -26077, -25831, -25582, 
	-25329, -25072, -24811, -24546, -24278, -24006, -23731, -23452, 
	-23169, -22883, -22594, -22301, -22004, -21705, -21402, -21096, 
	-20787, -20474, -20159, -19840, -19519, -19194, -18867, -18537, 
	-18204, -17868, -17530, -17189, -16845, -16499, -16150, -15799, 
	-15446, -15090, -14732, -14372, -14009, -13645, -13278, -12909, 
	-12539, -12166, -11792, -11416, -11038, -10659, -10278,  -9895, 
	 -9511,  -9126,  -8739,  -8351,  -7961,  -7571,  -7179,  -6786, 
	 -6392,  -5997,  -5601,  -5205,  -4807,  -4409,  -4011,  -3611, 
	 -3211,  -2811,  -2410,  -2009,  -1607,  -1206,   -804,   -402, 
	     0,    402,    804,   1206,   1607,   2009,   2410,   2811, 
	  3211,   3611,   4011,   4409,   4807,   5205,   5601,   5997, 
	  6392,   6786,   7179,   7571,   7961,   8351,   8739,   9126, 
	  9511,   9895,  10278,  10659,  11038,  11416,  11792,  12166, 
	 12539,  12909,  13278,  13645,  14009,  14372,  14732,  15090, 
	 15446,  15799,  16150,  16499,  16845,  17189,  17530,  17868, 
	 18204,  18537,  18867,  19194,  19519,  19840,  20159,  20474, 
	 20787,  21096,  21402,  21705,  22004,  22301,  22594,  22883, 
	 23169,  23452,  23731,  24006,  24278,  24546,  24811,  25072, 
	 25329,  25582,  25831,  26077,  26318,  26556,  26789,  27019, 
	 27244,  27466,  27683,  27896,  28105,  28309,  28510,  28706, 
	 28897,  29085,  29268,  29446,  29621,  29790,  29955,  30116, 
	 30272,  30424,  30571,  30713,  30851,  30984,  31113,  31236, 
	 31356,  31470,  31580,  31684,  31785,  31880,  31970,  32056, 
	 32137,  32213,  32284,  32350,  32412,  32468,  32520,  32567, 
	 32609,  32646,  32678,  32705,  32727,  32744,  32757,  32764
};

#define COS(x) (costab[(((x)>>7)&0x1ff)])
#define SIN(x) COS((x)+0xc000)
