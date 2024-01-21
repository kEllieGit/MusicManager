#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Sound/SoundCue.h"
#include "Chaos/Pair.h"
#include "MusicManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSongInterrupted, USoundCue*, song, bool, keepPaused);

UCLASS(Blueprintable, BlueprintType)
class MUSICMANAGER_API UMusicManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // Begin USubsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    // End USubsystem Interface

    /// @brief Changes the background song and plays it.
    /// @param song Song to play.
    /// @param startTime Time of the song to start playing at.
    UFUNCTION(BlueprintCallable, Category = "Music")
    void Play(USoundCue* song, float startTime = 0.0f);

    /// @brief Changes the background song and fades it in. If changing the active song, will also fade it out.
    /// @param song Song to play.
    /// @param fadeDuration Duration for the fade-in/fade-out effect.
    /// @param overlap 
    UFUNCTION(BlueprintCallable, Category = "Music")
    void FadeInPlay(USoundCue* song, float fadeDuration, bool overlap = false);

    /// @brief Changes the active background song and plays a chime before playing it.
    /// @param song Song to play.
    /// @param chime Chime to play before the new background song.
    /// @param startTime Time of the song to start playing at.
    UFUNCTION(BlueprintCallable, Category = "Music")
    void ChimeInPlay(USoundCue* song, USoundCue* chime, float startTime = 0.0f);

    /// @brief Interrupts the active background song with the provided song.
    /// @param song Song to interrupt the active background song with.
    /// @param keepPaused If true, keeps the background song paused until resumed.
    UFUNCTION(BlueprintCallable, Category = "Music")
    void Interrupt(USoundCue* song, bool keepPaused = false);

    /// @brief Clears all previously stacked interruptions.
    UFUNCTION(BlueprintCallable, Category = "Music")
    void ClearInterruptions();

    /// @brief Restarts the active background Song.
    UFUNCTION(BlueprintCallable, Category = "Music")
    void Restart(float fadeOutTime = 0.0f);

    /// @brief Stops playback of the active background song.
    UFUNCTION(BlueprintCallable, Category = "Music")
    void Stop();

    /// @brief Stops playback of the active background song after a delay.
    UFUNCTION(BlueprintCallable, Category = "Music")
    void StopDelayed(float delay);

    /// @brief Pauses the playback of the active background song.
    UFUNCTION(BlueprintCallable, Category = "Music")
    void Pause();

    /// @brief Resumes playback of the active background song after a pause.
    UFUNCTION(BlueprintCallable, Category = "Music")
    void Resume();

    /// @brief Sets the volume of the active background song.
    /// @param volume Volume to switch to.
    UFUNCTION(BlueprintCallable, Category = "Music")
    void SetVolume(float volume);

    /// @brief Sets the pitch of the active background song.
    /// @param pitch Pitch to switch to.
    UFUNCTION(BlueprintCallable, Category = "Music")
    void SetPitch(float pitch);

    /// @brief Plays the active background song from the given time.
    /// @param newTime Time to play at.
    UFUNCTION(BlueprintCallable, Category = "Music")
    void SetTime(float newTime);

    /// @brief Returns whether there is a valid active background song.
    UFUNCTION(BlueprintCallable, Category = "Music")
    bool IsSongValid() const;

    /// @brief Returns the elapsed playback time of the active background song.
    UFUNCTION(BlueprintCallable, Category = "Music")
    float GetSongTime() const;

    /// @brief Returns the duration of the active background song.
    UFUNCTION(BlueprintCallable, Category = "Music")
    float GetSongDuration() const;

    /// @brief Returns the active background song, if any.
    UFUNCTION(BlueprintCallable, Category = "Music")
    USoundCue* GetSong() const;

    /// @brief Delegate that gets called when the song has finished being interrupted.
    UPROPERTY(BlueprintAssignable)
    FSongInterrupted SongInterrupted;
private:
    /// @brief Internal method which plays the provided song.
    /// @param song Song to play.
    /// @param startTime Time of the song to start playing at.
    void PlaySong(USoundCue* song, float startTime = 0.0f);

    /// @brief Internal method used for playing the provided interruption.
    void PlayInterruption(TPair<USoundCue*, bool> interruption);

    /// @brief Returns whether the World instance for this MusicManager is valid.
    bool IsWorldValid();

    /// @brief Array of all of the interruptions set to play.
    TArray<TPair<USoundCue*, bool>> Interruptions;

    /// @brief Handle to keep track of the playback position of the background song.
    FTimerHandle Timer;

    /// @brief World context required for the MusicManager to function.
    UWorld* World;

    /// @brief Component used to interface with the background song playback.
    UAudioComponent* AudioComponent;

    /// @brief Currently active background song, if any. Otherwise nullptr.
    USoundCue* BackgroundSong;
};