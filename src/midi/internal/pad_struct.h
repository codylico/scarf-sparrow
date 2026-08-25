
#if !(defined hg_ScarfSparrowMidi_padStruct_h_)
#define hg_ScarfSparrowMidi_padStruct_h_

struct ALLEGRO_THREAD;

struct SSMIDI_PAD
{
  struct ALLEGRO_THREAD* worker;
  int done;
};

int ssmidi_init_pad(struct SSMIDI_PAD* pad, char const* addr);
void ssmidi_shutdown_pad(struct SSMIDI_PAD* pad);
void ssmidi_fast_pad_cleanup(struct SSMIDI_PAD* pad);

#endif //hg_ScarfSparrowMidi_padStruct_h_
