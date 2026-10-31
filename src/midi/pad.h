
#if !(defined hg_ScarfSparrowMidi_pad_h_)
#define hg_ScarfSparrowMidi_pad_h_

#if (defined _WIN32) && (defined ScarfSparrowMIDI_SHARED)
#  if (defined ScarfSparrowMIDI_WantExport)
#    define ScarfSparrowMIDI_EXPORT __declspec(dllexport)
#  else
#    define ScarfSparrowMIDI_EXPORT __declspec(dllimport)
#  endif //ScarfSparrowMIDI_WantExport
#else
#  define ScarfSparrowMIDI_EXPORT
#endif //_WIN32

#if (defined __cplusplus)
extern "C" {
#endif //__cplusplus

struct SSMIDI_PAD;
struct ALLEGRO_EVENT_SOURCE;

#define SSMIDI_PAD_EVENT_TYPE 0x53534D25

struct SSMIDI_PAD_NOTE
{
  unsigned int timestamp;
  unsigned char cmd;
  unsigned char args[3];
};

ScarfSparrowMIDI_EXPORT struct SSMIDI_PAD* ssmidi_create_pad(char const* addr);
ScarfSparrowMIDI_EXPORT void ssmidi_destroy_pad(struct SSMIDI_PAD* ptr);
ScarfSparrowMIDI_EXPORT int ssmidi_pad_get_note(struct SSMIDI_PAD* pad, struct SSMIDI_PAD_NOTE* pack);
ScarfSparrowMIDI_EXPORT struct ALLEGRO_EVENT_SOURCE* ssmidi_pad_get_event_source(struct SSMIDI_PAD* pad);

#if (defined __cplusplus)
};
#endif //__cplusplus

#endif //hg_ScarfSparrowMidi_pad_h_
