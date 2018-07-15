// qtutil.cc
// code for qtutil.h

#include "qtutil.h"                    // this module

// smase
#include "datablok.h"                  // DataBlock
#include "exc.h"                       // xassert

// Qt
#include <QByteArray>
#include <QObject>
#include <QPoint>
#include <QRect>
#include <QSize>

// libc
#include <stdio.h>                     // sprintf


// The numeric value and name of some enumerator flag.
template <class T>
struct FlagDefinition {
  T value;
  char const *name;
};


// If 'flags' contains 'flag.value', add its name to 'sb' and remove
// its value from 'flags'.
template <class T>
static void handleFlag(stringBuilder &sb, QFlags<T> &flags,
                       FlagDefinition<T> const &flag)
{
  if (flags & flag.value) {
    if (sb.length() > 0) {
      sb << "+";
    }
    sb << flag.name;
    flags ^= flag.value;
  }
}


// Render 'flags' as a string by using 'definitions' to decode it.
template <class T>
static string flagsToString(QFlags<T> flags,
                            FlagDefinition<T> const *definitions,
                            int numDefinitions,
                            char const *noFlagsName)
{
  stringBuilder sb;

  for (int i=0; i < numDefinitions; i++) {
    handleFlag(sb, flags, definitions[i]);
  }

  if (flags) {
    if (sb.length() > 0) {
      sb << " (plus unknown flags: " << (int)flags << ")";
    }
    else {
      sb << "(unknown flags: " << (int)flags << ")";
    }
  }

  if (sb.length() == 0) {
    sb << noFlagsName;
  }

  return sb;
}


#define FLAG_DEFN(flag) { Qt::flag, #flag },

static FlagDefinition<Qt::MouseButton> const mouseButtonDefinitions[] = {
  FLAG_DEFN(LeftButton)
  FLAG_DEFN(RightButton)
  FLAG_DEFN(MiddleButton)
  FLAG_DEFN(BackButton)
  FLAG_DEFN(ForwardButton)
  FLAG_DEFN(TaskButton)
  FLAG_DEFN(ExtraButton4)
  FLAG_DEFN(ExtraButton5)
  // ExtraButtons up to 24 are defined, but I'll stop here.
};

string toString(Qt::MouseButtons buttons)
{
  return flagsToString<Qt::MouseButton>(
    buttons,
    mouseButtonDefinitions,
    TABLESIZE(mouseButtonDefinitions),
    "NoButton");
}


static FlagDefinition<Qt::KeyboardModifier> const keyboardModifierDefinitions[] = {
  FLAG_DEFN(ShiftModifier)
  FLAG_DEFN(ControlModifier)
  FLAG_DEFN(AltModifier)
  FLAG_DEFN(MetaModifier)
  FLAG_DEFN(KeypadModifier)
  FLAG_DEFN(GroupSwitchModifier)
};

string toString(Qt::KeyboardModifiers kmods)
{
  return flagsToString<Qt::KeyboardModifier>(
    kmods,
    keyboardModifierDefinitions,
    TABLESIZE(keyboardModifierDefinitions),
    "NoModifier");
}


string toString(QPoint p)
{
  return stringb('(' << p.x() << ',' << p.y() << ')');
}


string toString(QRect r)
{
  return stringb('[' << toString(r.topLeft()) << '+' <<
                 toString(r.size()) << ']');
}


string toString(QSize s)
{
  return stringb('(' << s.width() << ',' << s.height() << ')');
}


string qrgbToString(QRgb rgba)
{
  char tmp[10];
  int n = sprintf(tmp, "#%08X", (unsigned int)rgba);
  xassert(n < TABLESIZE(tmp));
  return string(tmp);
}


string toString(QString const &s)
{
  return string(s.toUtf8().constData());
}


QString toQString(string const &s)
{
  return QString(s.c_str());
}


string qObjectDesc(QObject *obj)
{
  if (obj) {
    return stringb((void*)obj << " (" <<
                   toString(obj->objectName()) << ')');
  }
  else {
    return "null";
  }
}


void printQByteArray(QByteArray const &ba, char const *label)
{
  // This is an inefficient but convenient implementation.
  DataBlock db(ba.constData(), ba.size());
  db.print(label);
}


char const *toString(Qt::Key k)
{
  // This list is derived from qtbase/src/corelib/global/qnamespace.h
  // from Qt 5.9.

  #define HANDLE_KEY(key) \
    case Qt::key: return #key;

  switch (k) {
    HANDLE_KEY(Key_Escape)
    HANDLE_KEY(Key_Tab)
    HANDLE_KEY(Key_Backtab)
    HANDLE_KEY(Key_Backspace)
    HANDLE_KEY(Key_Return)
    HANDLE_KEY(Key_Enter)
    HANDLE_KEY(Key_Insert)
    HANDLE_KEY(Key_Delete)
    HANDLE_KEY(Key_Pause)
    HANDLE_KEY(Key_Print)
    HANDLE_KEY(Key_SysReq)
    HANDLE_KEY(Key_Clear)
    HANDLE_KEY(Key_Home)
    HANDLE_KEY(Key_End)
    HANDLE_KEY(Key_Left)
    HANDLE_KEY(Key_Up)
    HANDLE_KEY(Key_Right)
    HANDLE_KEY(Key_Down)
    HANDLE_KEY(Key_PageUp)
    HANDLE_KEY(Key_PageDown)
    HANDLE_KEY(Key_Shift)
    HANDLE_KEY(Key_Control)
    HANDLE_KEY(Key_Meta)
    HANDLE_KEY(Key_Alt)
    HANDLE_KEY(Key_CapsLock)
    HANDLE_KEY(Key_NumLock)
    HANDLE_KEY(Key_ScrollLock)
    HANDLE_KEY(Key_F1)
    HANDLE_KEY(Key_F2)
    HANDLE_KEY(Key_F3)
    HANDLE_KEY(Key_F4)
    HANDLE_KEY(Key_F5)
    HANDLE_KEY(Key_F6)
    HANDLE_KEY(Key_F7)
    HANDLE_KEY(Key_F8)
    HANDLE_KEY(Key_F9)
    HANDLE_KEY(Key_F10)
    HANDLE_KEY(Key_F11)
    HANDLE_KEY(Key_F12)
    HANDLE_KEY(Key_F13)
    HANDLE_KEY(Key_F14)
    HANDLE_KEY(Key_F15)
    HANDLE_KEY(Key_F16)
    HANDLE_KEY(Key_F17)
    HANDLE_KEY(Key_F18)
    HANDLE_KEY(Key_F19)
    HANDLE_KEY(Key_F20)
    HANDLE_KEY(Key_F21)
    HANDLE_KEY(Key_F22)
    HANDLE_KEY(Key_F23)
    HANDLE_KEY(Key_F24)
    HANDLE_KEY(Key_F25)
    HANDLE_KEY(Key_F26)
    HANDLE_KEY(Key_F27)
    HANDLE_KEY(Key_F28)
    HANDLE_KEY(Key_F29)
    HANDLE_KEY(Key_F30)
    HANDLE_KEY(Key_F31)
    HANDLE_KEY(Key_F32)
    HANDLE_KEY(Key_F33)
    HANDLE_KEY(Key_F34)
    HANDLE_KEY(Key_F35)
    HANDLE_KEY(Key_Super_L)
    HANDLE_KEY(Key_Super_R)
    HANDLE_KEY(Key_Menu)
    HANDLE_KEY(Key_Hyper_L)
    HANDLE_KEY(Key_Hyper_R)
    HANDLE_KEY(Key_Help)
    HANDLE_KEY(Key_Direction_L)
    HANDLE_KEY(Key_Direction_R)
    HANDLE_KEY(Key_Space)
    //HANDLE_KEY(Key_Any)  // Weird.  Duplicate.
    HANDLE_KEY(Key_Exclam)
    HANDLE_KEY(Key_QuoteDbl)
    HANDLE_KEY(Key_NumberSign)
    HANDLE_KEY(Key_Dollar)
    HANDLE_KEY(Key_Percent)
    HANDLE_KEY(Key_Ampersand)
    HANDLE_KEY(Key_Apostrophe)
    HANDLE_KEY(Key_ParenLeft)
    HANDLE_KEY(Key_ParenRight)
    HANDLE_KEY(Key_Asterisk)
    HANDLE_KEY(Key_Plus)
    HANDLE_KEY(Key_Comma)
    HANDLE_KEY(Key_Minus)
    HANDLE_KEY(Key_Period)
    HANDLE_KEY(Key_Slash)
    HANDLE_KEY(Key_0)
    HANDLE_KEY(Key_1)
    HANDLE_KEY(Key_2)
    HANDLE_KEY(Key_3)
    HANDLE_KEY(Key_4)
    HANDLE_KEY(Key_5)
    HANDLE_KEY(Key_6)
    HANDLE_KEY(Key_7)
    HANDLE_KEY(Key_8)
    HANDLE_KEY(Key_9)
    HANDLE_KEY(Key_Colon)
    HANDLE_KEY(Key_Semicolon)
    HANDLE_KEY(Key_Less)
    HANDLE_KEY(Key_Equal)
    HANDLE_KEY(Key_Greater)
    HANDLE_KEY(Key_Question)
    HANDLE_KEY(Key_At)
    HANDLE_KEY(Key_A)
    HANDLE_KEY(Key_B)
    HANDLE_KEY(Key_C)
    HANDLE_KEY(Key_D)
    HANDLE_KEY(Key_E)
    HANDLE_KEY(Key_F)
    HANDLE_KEY(Key_G)
    HANDLE_KEY(Key_H)
    HANDLE_KEY(Key_I)
    HANDLE_KEY(Key_J)
    HANDLE_KEY(Key_K)
    HANDLE_KEY(Key_L)
    HANDLE_KEY(Key_M)
    HANDLE_KEY(Key_N)
    HANDLE_KEY(Key_O)
    HANDLE_KEY(Key_P)
    HANDLE_KEY(Key_Q)
    HANDLE_KEY(Key_R)
    HANDLE_KEY(Key_S)
    HANDLE_KEY(Key_T)
    HANDLE_KEY(Key_U)
    HANDLE_KEY(Key_V)
    HANDLE_KEY(Key_W)
    HANDLE_KEY(Key_X)
    HANDLE_KEY(Key_Y)
    HANDLE_KEY(Key_Z)
    HANDLE_KEY(Key_BracketLeft)
    HANDLE_KEY(Key_Backslash)
    HANDLE_KEY(Key_BracketRight)
    HANDLE_KEY(Key_AsciiCircum)
    HANDLE_KEY(Key_Underscore)
    HANDLE_KEY(Key_QuoteLeft)
    HANDLE_KEY(Key_BraceLeft)
    HANDLE_KEY(Key_Bar)
    HANDLE_KEY(Key_BraceRight)
    HANDLE_KEY(Key_AsciiTilde)
    HANDLE_KEY(Key_nobreakspace)
    HANDLE_KEY(Key_exclamdown)
    HANDLE_KEY(Key_cent)
    HANDLE_KEY(Key_sterling)
    HANDLE_KEY(Key_currency)
    HANDLE_KEY(Key_yen)
    HANDLE_KEY(Key_brokenbar)
    HANDLE_KEY(Key_section)
    HANDLE_KEY(Key_diaeresis)
    HANDLE_KEY(Key_copyright)
    HANDLE_KEY(Key_ordfeminine)
    HANDLE_KEY(Key_guillemotleft)
    HANDLE_KEY(Key_notsign)
    HANDLE_KEY(Key_hyphen)
    HANDLE_KEY(Key_registered)
    HANDLE_KEY(Key_macron)
    HANDLE_KEY(Key_degree)
    HANDLE_KEY(Key_plusminus)
    HANDLE_KEY(Key_twosuperior)
    HANDLE_KEY(Key_threesuperior)
    HANDLE_KEY(Key_acute)
    HANDLE_KEY(Key_mu)
    HANDLE_KEY(Key_paragraph)
    HANDLE_KEY(Key_periodcentered)
    HANDLE_KEY(Key_cedilla)
    HANDLE_KEY(Key_onesuperior)
    HANDLE_KEY(Key_masculine)
    HANDLE_KEY(Key_guillemotright)
    HANDLE_KEY(Key_onequarter)
    HANDLE_KEY(Key_onehalf)
    HANDLE_KEY(Key_threequarters)
    HANDLE_KEY(Key_questiondown)
    HANDLE_KEY(Key_Agrave)
    HANDLE_KEY(Key_Aacute)
    HANDLE_KEY(Key_Acircumflex)
    HANDLE_KEY(Key_Atilde)
    HANDLE_KEY(Key_Adiaeresis)
    HANDLE_KEY(Key_Aring)
    HANDLE_KEY(Key_AE)
    HANDLE_KEY(Key_Ccedilla)
    HANDLE_KEY(Key_Egrave)
    HANDLE_KEY(Key_Eacute)
    HANDLE_KEY(Key_Ecircumflex)
    HANDLE_KEY(Key_Ediaeresis)
    HANDLE_KEY(Key_Igrave)
    HANDLE_KEY(Key_Iacute)
    HANDLE_KEY(Key_Icircumflex)
    HANDLE_KEY(Key_Idiaeresis)
    HANDLE_KEY(Key_ETH)
    HANDLE_KEY(Key_Ntilde)
    HANDLE_KEY(Key_Ograve)
    HANDLE_KEY(Key_Oacute)
    HANDLE_KEY(Key_Ocircumflex)
    HANDLE_KEY(Key_Otilde)
    HANDLE_KEY(Key_Odiaeresis)
    HANDLE_KEY(Key_multiply)
    HANDLE_KEY(Key_Ooblique)
    HANDLE_KEY(Key_Ugrave)
    HANDLE_KEY(Key_Uacute)
    HANDLE_KEY(Key_Ucircumflex)
    HANDLE_KEY(Key_Udiaeresis)
    HANDLE_KEY(Key_Yacute)
    HANDLE_KEY(Key_THORN)
    HANDLE_KEY(Key_ssharp)
    HANDLE_KEY(Key_division)
    HANDLE_KEY(Key_ydiaeresis)
    HANDLE_KEY(Key_AltGr)
    HANDLE_KEY(Key_Multi_key)
    HANDLE_KEY(Key_Codeinput)
    HANDLE_KEY(Key_SingleCandidate)
    HANDLE_KEY(Key_MultipleCandidate)
    HANDLE_KEY(Key_PreviousCandidate)
    HANDLE_KEY(Key_Mode_switch)
    HANDLE_KEY(Key_Kanji)
    HANDLE_KEY(Key_Muhenkan)
    HANDLE_KEY(Key_Henkan)
    HANDLE_KEY(Key_Romaji)
    HANDLE_KEY(Key_Hiragana)
    HANDLE_KEY(Key_Katakana)
    HANDLE_KEY(Key_Hiragana_Katakana)
    HANDLE_KEY(Key_Zenkaku)
    HANDLE_KEY(Key_Hankaku)
    HANDLE_KEY(Key_Zenkaku_Hankaku)
    HANDLE_KEY(Key_Touroku)
    HANDLE_KEY(Key_Massyo)
    HANDLE_KEY(Key_Kana_Lock)
    HANDLE_KEY(Key_Kana_Shift)
    HANDLE_KEY(Key_Eisu_Shift)
    HANDLE_KEY(Key_Eisu_toggle)
    HANDLE_KEY(Key_Hangul)
    HANDLE_KEY(Key_Hangul_Start)
    HANDLE_KEY(Key_Hangul_End)
    HANDLE_KEY(Key_Hangul_Hanja)
    HANDLE_KEY(Key_Hangul_Jamo)
    HANDLE_KEY(Key_Hangul_Romaja)
    HANDLE_KEY(Key_Hangul_Jeonja)
    HANDLE_KEY(Key_Hangul_Banja)
    HANDLE_KEY(Key_Hangul_PreHanja)
    HANDLE_KEY(Key_Hangul_PostHanja)
    HANDLE_KEY(Key_Hangul_Special)
    HANDLE_KEY(Key_Dead_Grave)
    HANDLE_KEY(Key_Dead_Acute)
    HANDLE_KEY(Key_Dead_Circumflex)
    HANDLE_KEY(Key_Dead_Tilde)
    HANDLE_KEY(Key_Dead_Macron)
    HANDLE_KEY(Key_Dead_Breve)
    HANDLE_KEY(Key_Dead_Abovedot)
    HANDLE_KEY(Key_Dead_Diaeresis)
    HANDLE_KEY(Key_Dead_Abovering)
    HANDLE_KEY(Key_Dead_Doubleacute)
    HANDLE_KEY(Key_Dead_Caron)
    HANDLE_KEY(Key_Dead_Cedilla)
    HANDLE_KEY(Key_Dead_Ogonek)
    HANDLE_KEY(Key_Dead_Iota)
    HANDLE_KEY(Key_Dead_Voiced_Sound)
    HANDLE_KEY(Key_Dead_Semivoiced_Sound)
    HANDLE_KEY(Key_Dead_Belowdot)
    HANDLE_KEY(Key_Dead_Hook)
    HANDLE_KEY(Key_Dead_Horn)
    HANDLE_KEY(Key_Back)
    HANDLE_KEY(Key_Forward)
    HANDLE_KEY(Key_Stop)
    HANDLE_KEY(Key_Refresh)
    HANDLE_KEY(Key_VolumeDown)
    HANDLE_KEY(Key_VolumeMute)
    HANDLE_KEY(Key_VolumeUp)
    HANDLE_KEY(Key_BassBoost)
    HANDLE_KEY(Key_BassUp)
    HANDLE_KEY(Key_BassDown)
    HANDLE_KEY(Key_TrebleUp)
    HANDLE_KEY(Key_TrebleDown)
    HANDLE_KEY(Key_MediaPlay)
    HANDLE_KEY(Key_MediaStop)
    HANDLE_KEY(Key_MediaPrevious)
    HANDLE_KEY(Key_MediaNext)
    HANDLE_KEY(Key_MediaRecord)
    HANDLE_KEY(Key_MediaPause)
    HANDLE_KEY(Key_MediaTogglePlayPause)
    HANDLE_KEY(Key_HomePage)
    HANDLE_KEY(Key_Favorites)
    HANDLE_KEY(Key_Search)
    HANDLE_KEY(Key_Standby)
    HANDLE_KEY(Key_OpenUrl)
    HANDLE_KEY(Key_LaunchMail)
    HANDLE_KEY(Key_LaunchMedia)
    HANDLE_KEY(Key_Launch0)
    HANDLE_KEY(Key_Launch1)
    HANDLE_KEY(Key_Launch2)
    HANDLE_KEY(Key_Launch3)
    HANDLE_KEY(Key_Launch4)
    HANDLE_KEY(Key_Launch5)
    HANDLE_KEY(Key_Launch6)
    HANDLE_KEY(Key_Launch7)
    HANDLE_KEY(Key_Launch8)
    HANDLE_KEY(Key_Launch9)
    HANDLE_KEY(Key_LaunchA)
    HANDLE_KEY(Key_LaunchB)
    HANDLE_KEY(Key_LaunchC)
    HANDLE_KEY(Key_LaunchD)
    HANDLE_KEY(Key_LaunchE)
    HANDLE_KEY(Key_LaunchF)
    HANDLE_KEY(Key_MonBrightnessUp)
    HANDLE_KEY(Key_MonBrightnessDown)
    HANDLE_KEY(Key_KeyboardLightOnOff)
    HANDLE_KEY(Key_KeyboardBrightnessUp)
    HANDLE_KEY(Key_KeyboardBrightnessDown)
    HANDLE_KEY(Key_PowerOff)
    HANDLE_KEY(Key_WakeUp)
    HANDLE_KEY(Key_Eject)
    HANDLE_KEY(Key_ScreenSaver)
    HANDLE_KEY(Key_WWW)
    HANDLE_KEY(Key_Memo)
    HANDLE_KEY(Key_LightBulb)
    HANDLE_KEY(Key_Shop)
    HANDLE_KEY(Key_History)
    HANDLE_KEY(Key_AddFavorite)
    HANDLE_KEY(Key_HotLinks)
    HANDLE_KEY(Key_BrightnessAdjust)
    HANDLE_KEY(Key_Finance)
    HANDLE_KEY(Key_Community)
    HANDLE_KEY(Key_AudioRewind)
    HANDLE_KEY(Key_BackForward)
    HANDLE_KEY(Key_ApplicationLeft)
    HANDLE_KEY(Key_ApplicationRight)
    HANDLE_KEY(Key_Book)
    HANDLE_KEY(Key_CD)
    HANDLE_KEY(Key_Calculator)
    HANDLE_KEY(Key_ToDoList)
    HANDLE_KEY(Key_ClearGrab)
    HANDLE_KEY(Key_Close)
    HANDLE_KEY(Key_Copy)
    HANDLE_KEY(Key_Cut)
    HANDLE_KEY(Key_Display)
    HANDLE_KEY(Key_DOS)
    HANDLE_KEY(Key_Documents)
    HANDLE_KEY(Key_Excel)
    HANDLE_KEY(Key_Explorer)
    HANDLE_KEY(Key_Game)
    HANDLE_KEY(Key_Go)
    HANDLE_KEY(Key_iTouch)
    HANDLE_KEY(Key_LogOff)
    HANDLE_KEY(Key_Market)
    HANDLE_KEY(Key_Meeting)
    HANDLE_KEY(Key_MenuKB)
    HANDLE_KEY(Key_MenuPB)
    HANDLE_KEY(Key_MySites)
    HANDLE_KEY(Key_News)
    HANDLE_KEY(Key_OfficeHome)
    HANDLE_KEY(Key_Option)
    HANDLE_KEY(Key_Paste)
    HANDLE_KEY(Key_Phone)
    HANDLE_KEY(Key_Calendar)
    HANDLE_KEY(Key_Reply)
    HANDLE_KEY(Key_Reload)
    HANDLE_KEY(Key_RotateWindows)
    HANDLE_KEY(Key_RotationPB)
    HANDLE_KEY(Key_RotationKB)
    HANDLE_KEY(Key_Save)
    HANDLE_KEY(Key_Send)
    HANDLE_KEY(Key_Spell)
    HANDLE_KEY(Key_SplitScreen)
    HANDLE_KEY(Key_Support)
    HANDLE_KEY(Key_TaskPane)
    HANDLE_KEY(Key_Terminal)
    HANDLE_KEY(Key_Tools)
    HANDLE_KEY(Key_Travel)
    HANDLE_KEY(Key_Video)
    HANDLE_KEY(Key_Word)
    HANDLE_KEY(Key_Xfer)
    HANDLE_KEY(Key_ZoomIn)
    HANDLE_KEY(Key_ZoomOut)
    HANDLE_KEY(Key_Away)
    HANDLE_KEY(Key_Messenger)
    HANDLE_KEY(Key_WebCam)
    HANDLE_KEY(Key_MailForward)
    HANDLE_KEY(Key_Pictures)
    HANDLE_KEY(Key_Music)
    HANDLE_KEY(Key_Battery)
    HANDLE_KEY(Key_Bluetooth)
    HANDLE_KEY(Key_WLAN)
    HANDLE_KEY(Key_UWB)
    HANDLE_KEY(Key_AudioForward)
    HANDLE_KEY(Key_AudioRepeat)
    HANDLE_KEY(Key_AudioRandomPlay)
    HANDLE_KEY(Key_Subtitle)
    HANDLE_KEY(Key_AudioCycleTrack)
    HANDLE_KEY(Key_Time)
    HANDLE_KEY(Key_Hibernate)
    HANDLE_KEY(Key_View)
    HANDLE_KEY(Key_TopMenu)
    HANDLE_KEY(Key_PowerDown)
    HANDLE_KEY(Key_Suspend)
    HANDLE_KEY(Key_ContrastAdjust)
    HANDLE_KEY(Key_LaunchG)
    HANDLE_KEY(Key_LaunchH)
    HANDLE_KEY(Key_TouchpadToggle)
    HANDLE_KEY(Key_TouchpadOn)
    HANDLE_KEY(Key_TouchpadOff)
    HANDLE_KEY(Key_MicMute)
    HANDLE_KEY(Key_Red)
    HANDLE_KEY(Key_Green)
    HANDLE_KEY(Key_Yellow)
    HANDLE_KEY(Key_Blue)
    HANDLE_KEY(Key_ChannelUp)
    HANDLE_KEY(Key_ChannelDown)
    HANDLE_KEY(Key_Guide)
    HANDLE_KEY(Key_Info)
    HANDLE_KEY(Key_Settings)
    HANDLE_KEY(Key_MicVolumeUp)
    HANDLE_KEY(Key_MicVolumeDown)
    HANDLE_KEY(Key_New)
    HANDLE_KEY(Key_Open)
    HANDLE_KEY(Key_Find)
    HANDLE_KEY(Key_Undo)
    HANDLE_KEY(Key_Redo)
    HANDLE_KEY(Key_MediaLast)
    HANDLE_KEY(Key_Select)
    HANDLE_KEY(Key_Yes)
    HANDLE_KEY(Key_No)
    HANDLE_KEY(Key_Cancel)
    HANDLE_KEY(Key_Printer)
    HANDLE_KEY(Key_Execute)
    HANDLE_KEY(Key_Sleep)
    HANDLE_KEY(Key_Play)
    HANDLE_KEY(Key_Zoom)
    HANDLE_KEY(Key_Exit)
    HANDLE_KEY(Key_Context1)
    HANDLE_KEY(Key_Context2)
    HANDLE_KEY(Key_Context3)
    HANDLE_KEY(Key_Context4)
    HANDLE_KEY(Key_Call)
    HANDLE_KEY(Key_Hangup)
    HANDLE_KEY(Key_Flip)
    HANDLE_KEY(Key_ToggleCallHangup)
    HANDLE_KEY(Key_VoiceDial)
    HANDLE_KEY(Key_LastNumberRedial)
    HANDLE_KEY(Key_Camera)
    HANDLE_KEY(Key_CameraFocus)
    HANDLE_KEY(Key_unknown)

    default: return "(unknown)";
  }

  #undef HANDLE_KEY
}


// EOF
