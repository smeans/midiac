const int song_note_count = sizeof(song_buffer)/sizeof(*song_buffer);

int current_note_page = -1;
const int note_page_count = 512;
NOTE note_page[note_page_count];

inline NOTE *PSONG(unsigned int note_index) {
  int target_note_page = note_index / note_page_count;
  if (current_note_page != target_note_page) {
    current_note_page = target_note_page;
    unsigned int note_page_base = current_note_page * note_page_count;
    memcpy_P(note_page, &song_buffer[note_page_base], min(song_note_count-note_page_base, note_page_count) * sizeof(*note_page));
  }

  return &note_page[note_index % note_page_count];
}

#define PSONG_COUNT song_note_count
