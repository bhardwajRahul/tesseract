/**********************************************************************
 * File:        tface.cpp  (Formerly tface.c)
 * Description: C side of the Tess/tessedit C/C++ interface.
 * Author:      Ray Smith
 *
 * (C) Copyright 1992, Hewlett-Packard Ltd.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 *
 **********************************************************************/

#include <cmath>

#include "wordrec.h"

#ifndef DISABLED_LEGACY_ENGINE
#  include "chop.h"
#  include "featdefs.h"
#  include "pageres.h"
#  include "params_model.h"
#endif

namespace tesseract {

/**
 * @name program_editup
 *
 * Initialize all the things in the program that need to be initialized.
 * init_permute determines whether to initialize the permute functions
 * and Dawg models.
 */
void Wordrec::program_editup(const std::string &textbase, TessdataManager *init_classifier,
                             TessdataManager *init_dict) {
  if (!textbase.empty()) {
    imagefile = textbase;
  }
#ifndef DISABLED_LEGACY_ENGINE
  InitFeatureDefs(&feature_defs_);
  InitAdaptiveClassifier(init_classifier);
  if (init_dict) {
    getDict().SetupForLoad(Dict::GlobalDawgCache());
    getDict().Load(lang, init_dict);
    getDict().FinishLoad();
  }
  pass2_ok_split = chop_ok_split;
#endif // ndef DISABLED_LEGACY_ENGINE
}

/**
 * @name end_recog
 *
 * Cleanup and exit the recog program.
 */
int Wordrec::end_recog() {
  program_editdown(0);

  return (0);
}

/**
 * @name program_editdown
 *
 * This function holds any necessary post processing for the Wise Owl
 * program.
 */
void Wordrec::program_editdown(int32_t elapsed_time) {
#ifndef DISABLED_LEGACY_ENGINE
  EndAdaptiveClassifier();
#endif // ndef DISABLED_LEGACY_ENGINE
  getDict().End();
}

/**
 * @name dict_word()
 *
 * Test the dictionaries, returning NO_PERM (0) if not found, or one
 * of the PermuterType values if found, according to the dictionary.
 */
int Wordrec::dict_word(const WERD_CHOICE &word) {
  return getDict().valid_word(word);
}

#ifndef DISABLED_LEGACY_ENGINE

/**
 * @name set_pass1
 *
 * Get ready to do some pass 1 stuff.
 */
void Wordrec::set_pass1() {
  chop_ok_split.set_value(70.0);
  language_model_->getParamsModel().SetPass(ParamsModel::PTRAIN_PASS1);
  SetupPass1();
}

/**
 * @name set_pass2
 *
 * Get ready to do some pass 2 stuff.
 */
void Wordrec::set_pass2() {
  chop_ok_split.set_value(pass2_ok_split);
  language_model_->getParamsModel().SetPass(ParamsModel::PTRAIN_PASS2);
  SetupPass2();
}

/**
 * @name cc_recog
 *
 * Recognize a word.
 */
void Wordrec::cc_recog(WERD_RES *word) {
  getDict().reset_hyphen_vars(word->word->flag(W_EOL));
  chop_word_main(word);
  word->DebugWordChoices(getDict().stopper_debug_level >= 1, getDict().word_to_debug.c_str());
  ASSERT_HOST(word->StatesAllValid());
}

/**
 * @name call_matcher
 *
 * Called from Tess with a blob in tess form.
 * The blob may need rotating to the correct orientation for classification.
 */
BLOB_CHOICE_LIST *Wordrec::call_matcher(TBLOB *tessblob) {
  // Rotate the blob for classification if necessary.
  TBLOB *rotated_blob = tessblob->ClassifyNormalizeIfNeeded();
  if (rotated_blob == nullptr) {
    rotated_blob = tessblob;
  }
  auto *ratings = new BLOB_CHOICE_LIST(); // matcher result
  AdaptiveClassifier(rotated_blob, ratings);
  if (rotated_blob != tessblob) {
    delete rotated_blob;
  }
  return ratings;
}

#endif // ndef DISABLED_LEGACY_ENGINE

} // namespace tesseract
