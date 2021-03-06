/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "AudioCtrlCs.h"
#include "RouteEnums.h"
#include "AudioClipsEnum.h"

/* ******************
 
This is the syntax implemented by the class AudioCtrlLanguageCs:

SoundListNormal := |
                   FirstCrossing
                   FirstCrossing AdditionalCrossing

FirstCrossing := ActionAndWhen

AdditionalCrossing := <AndThen> ActionAndWhen

SoundListPriority := |
                     <DuKorAtFelHall> |
                     <DuHarAvvikitFranRutten> |
                     <DuArFrammeVidDinDest>

ActionAndWhen := Action When |
                 Action

Action := <SvangVanster>      |
          <SvangHoger>        |
          <HallVanster>       |
          <HallHoger>         |
          <KorRaktFram>       |
          <VidDestinationen>  |
          <SvangVanster> <IRondellen> |
          <SvangHoger>   <IRondellen> |
          <KorRaktFram>  <IRondellen> |
          <KorUr>
          RoundaboutExit

RoundaboutExit := <KorUr> |
                  <Ta> NumberedExit <Utfarten> <IRondellen>

When := <Haer> |
        <Om[Distance]>  (Distance = 25, 50, 100, 200, 500 meter, 1, 2 km)


********************* */

namespace isab{
   using namespace RouteEnums;
   using namespace AudioClipsEnum;

   AudioCtrlLanguageCs::AudioCtrlLanguageCs() : AudioCtrlLanguageStd()
   {
   }

   class AudioCtrlLanguage* AudioCtrlLanguageCs::New()
   { 
      return new AudioCtrlLanguageCs(); 
   }

   int AudioCtrlLanguageCs::newCameraSoundList(
         DistanceInfo &deadlines,
         SoundClipsList &soundClips)
   {
      deadlines.sayAtDistance = -1;
      deadlines.abortTooShortDistance = -1;
      deadlines.abortTooFarDistance = -1;

      m_soundState.soundClips = &soundClips;
      m_soundState.truncPoint = 0;
      m_soundState.noMoreSounds = false;

      appendClip(CsSoundCamera);
      appendClip(SoundEnd);
      resetState();

      m_soundState.soundClips = NULL;
      return 0;
   }

   int AudioCtrlLanguageCs::supportsFeetMiles()
   {
      return 1;
   }

   int AudioCtrlLanguageCs::supportsYardsMiles()
   {
      return 1;
   }

   void AudioCtrlLanguageCs::Distance()
   {

      switch (m_nextXing.spokenDist) {
         /* Metric units */
         case 2000:
            appendClip(CsSoundPo2Kilometrech);
            break;
         case 1000:
            appendClip(CsSoundPo1Kilometru);
            break;
         case 500:
            appendClip(CsSoundPo500Metrech);
            break;
         case 200:
            appendClip(CsSoundPo200Metrech);
            break;
         case 100:
            appendClip(CsSoundPo100Metrech);
            break;
         case 50:
            appendClip(CsSoundPo50Metrech);
            break;
         case 25:
            appendClip(CsSoundPo25Metrech);
            break;

         /* Imperial units - miles */
         case 3219:
            appendClip(CsSoundPo2Milich);
            break;
         case 1609:
            appendClip(CsSoundPo1Mili);
            break;
         case 803:
            appendClip(CsSoundPoPulMile);
            break;
         case 402:
            appendClip(CsSoundPoCtvrtMile);
            break;

         /* Imperial units - feet */
         case 152:
            appendClip(CsSoundPo500Stopach);
            break;
         case 61:
            appendClip(CsSoundPo200Stopach);
            break;
         case 31:
            appendClip(CsSoundPo100Stopach);
            break;

         /* Imperial units - yards */
         case 182:
            appendClip(CsSoundPo200Yardech);
            break;
         case 91:
            appendClip(CsSoundPo100Yardech);
            break;
         case 46:
            appendClip(CsSoundPo50Yardech);
            break;
         case 23:
            appendClip(CsSoundPo25Yardech);
            break;

         /* Here */
         case 0:
            // Should never happen
            appendClip(CsSoundZde);
            break;
         default:
            // Internal error, kill the whole sound.
            truncateThisCrossing();
      }
   }

   void AudioCtrlLanguageCs::When()
   {
      if (m_nextXing.spokenDist == 0) {
         if (m_nextXing.crossingNum == 1) {
            appendClip(CsSoundZde);
         } else {
            appendClip(CsSoundIhned);
         }
      } else {
         Distance();
      }
      if (m_nextXing.setTimingMarker) {
         appendClip(SoundTimingMarker);
      }
   }

   void AudioCtrlLanguageCs::NumberedExit()
   {
      if ( (m_nextXing.xing->exitCount < 1) ||
           (m_nextXing.xing->exitCount > 8) ) {
         // Impossible, can not code that many exits - do nothing
         return;
      }
      appendClip(CsSoundPrvnim + m_nextXing.xing->exitCount - 1);
   }

   void AudioCtrlLanguageCs::RoundaboutExit()
   {
      if (m_nextXing.spokenDist == 0) {
         appendClip(CsSoundVyjedte);
      } else {
         appendClip(CsSoundJedte);
         NumberedExit();
         appendClip(CsSoundVyjezdem);
         appendClip(CsSoundNaKruhovemObjezdu);
      }
   }

   void AudioCtrlLanguageCs::ActionAndWhen()
   {
      if ( (m_nextXing.spokenDist == 0)                         ||
           (RouteAction(m_nextXing.xing->action) == Finally) 
         ) {
         Action();
         When();
      } else {
         When();
         Action();
      }
      if (m_nextXing.setTimingMarker) {
         appendClip(SoundTimingMarker);
      }
   }

   void AudioCtrlLanguageCs::Action()
   {
      switch (RouteAction(m_nextXing.xing->action)) {
         case InvalidAction:
         case Delta:
         case RouteActionMax:
         case End:
         case Start:
            /* Should never occur */
            truncateThisCrossing();
            return;
         case EnterRdbt:
         case Ahead:
         case On:   /* Enter highway from ramp */
         case ParkCar:
         case FollowRoad:
         case EnterFerry:
         case ChangeFerry:
            /* No sound on purpose. */
            truncateThisCrossing();
            return;
         case Left:
            appendClip(CsSoundOdbocteVlevo);
            break;
         case Right:
            appendClip(CsSoundOdbocteVpravo);
            break;
         case Finally:
            appendClip(CsSoundVCili);
                             // NB! No distance info added to this sound if spokenDist==0
            break;
         case ExitRdbt:
            RoundaboutExit();
            break;
         case AheadRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(CsSoundVyjedte);
            } else {
               appendClip(CsSoundJedtePrimo);
               appendClip(CsSoundNaKruhovemObjezdu);
            }
            break;
         case LeftRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(CsSoundVyjedte);
            } else {
               appendClip(CsSoundOdbocteVlevo);
               appendClip(CsSoundNaKruhovemObjezdu);
            }
            break;
         case RightRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(CsSoundVyjedte);
            } else {
               appendClip(CsSoundOdbocteVpravo);
               appendClip(CsSoundNaKruhovemObjezdu);
            }
            break;
         case ExitAt:
         case OffRampLeft:
         case OffRampRight:
            appendClip(CsSoundVyjedte);
            break;
         case KeepLeft:
            appendClip(CsSoundDrzteSeVlevo);
            break;
         case KeepRight:
            appendClip(CsSoundDrzteSeVpravo);
            break;
         case UTurn:
            appendClip(CsSoundProsimOtocteSe);
            break;
         case StartWithUTurn:
            appendClip(CsSoundOtocteSeAzJeToMozne);
            break;
         case UTurnRdbt:
            if (m_nextXing.spokenDist == 0) {
               appendClip(CsSoundVyjedte);
            } else {
               appendClip(CsSoundProsimOtocteSe);
               appendClip(CsSoundNaKruhovemObjezdu);
            }
            break;
            
         default: 
            // Make no sound for unknown turns.
            truncateThisCrossing();
            return;
      }
   }

   void AudioCtrlLanguageCs::FirstCrossing()
   {
      ActionAndWhen();
   }

   void AudioCtrlLanguageCs::AdditionalCrossing()
   {
      appendClip(CsSoundAPotom);
      ActionAndWhen();
   }

   void AudioCtrlLanguageCs::genericDeviatedFromRoute()
   {
      appendClip(CsSoundNyniJsteMimoTrasu);
   }

   void AudioCtrlLanguageCs::genericReachedDest()
   {
      appendClip(CsSoundJsteVCili);
   }

} /* namespace isab */


