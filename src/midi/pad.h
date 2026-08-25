
#if !(defined hg_ScarfSparrowMidi_pad_h_)
#define hg_ScarfSparrowMidi_pad_h_

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

struct SSMIDI_PAD* ssmidi_create_pad(char const* addr);
void ssmidi_destroy_pad(struct SSMIDI_PAD* ptr);
int ssmidi_pad_get_note(struct SSMIDI_PAD* pad, struct SSMIDI_PAD_NOTE* pack);
struct ALLEGRO_EVENT_SOURCE* ssmidi_pad_get_event_source(struct SSMIDI_PAD* pad);

#if (defined __cplusplus)
};
#endif //__cplusplus

#endif //hg_ScarfSparrowMidi_pad_h_
