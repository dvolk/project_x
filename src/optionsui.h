#pragma once

#include "./ui.h"
#include "./textbutton.h"
#include "./labelledcheckbox.h"
#include "./slider.h"

#include <vector>

struct OptionsUI : public UI {
    TextButton *button_cancel;
    TextButton *button_apply;

    std::vector<LabelledCheckBox *> checkboxes;
    std::vector<LabelledCheckBox *> resolution_checkboxes;

    LabelledCheckBox *fullscreenCB;
    LabelledCheckBox *vsyncCB;
    LabelledCheckBox *resolutionScalingCB;
    LabelledCheckBox *debugVisibilityCB;
    LabelledCheckBox *sortingCB;
    LabelledCheckBox *startNagCB;
    LabelledCheckBox *uiFadingCB;
    LabelledCheckBox *altGridMovementCB;
    LabelledCheckBox *playMusicCB;
    LabelledCheckBox *playUISoundsCB;
    LabelledCheckBox *showFPSCB;
    LabelledCheckBox *escMenuQuitsCB;
    LabelledCheckBox *logToFileCB;
    LabelledCheckBox *nativeSaveLoadDialogsCB;

    Slider *musicVolumeSlider;
    Slider *uiSoundVolumeSlider;

    LabelledCheckBox *clipRectangleCB;
    LabelledCheckBox *playerInvulnerableCB;

    void reset_settings(void);
    void apply_settings(void);

    OptionsUI();
    ~OptionsUI();
};
