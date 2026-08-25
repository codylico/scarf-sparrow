
#if !(defined hg_ScarfSparrowMidi_pad_h_)
#define hg_ScarfSparrowMidi_pad_h_

#if (defined __cplusplus)
extern "C" {
#endif //__cplusplus

struct SSMIDI_PAD;


struct SSMIDI_PAD* ssmidi_create_pad(char const* addr);
void ssmidi_destroy_pad(struct SSMIDI_PAD* ptr);

#if (defined __cplusplus)
};
#endif //__cplusplus

#endif //hg_ScarfSparrowMidi_pad_h_
