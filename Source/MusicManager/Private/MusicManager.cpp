#pragma once

#include "MusicManager.h"
#include "Logging.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

void UMusicManager::Initialize(FSubsystemCollectionBase& Collection)
{
    BackgroundSong = nullptr;
    AudioComponent = nullptr;
}

void UMusicManager::Deinitialize()
{
}

void UMusicManager::Play(USoundCue* song, float startTime)
{
    if (!IsWorldValid())
    {
        UE_LOG(LogMusicManager, Error, TEXT("Playing songs without a world is not possible!"));
        return;
    }

    if (song == nullptr)
    {
        UE_LOG(LogMusicManager, Error, TEXT("Unable to play song, song is nullptr!"));
        return;
    }

    if (BackgroundSong == nullptr)
    {
        BackgroundSong = song;
    }
    else
    {
        Stop();
        BackgroundSong = song;
        World->GetTimerManager().ClearTimer(Timer);
    }

    PlaySong(song, startTime);
    World->GetTimerManager().SetTimer(Timer, 0.001f, true);
}

void UMusicManager::FadeInPlay(USoundCue* song, float fadeDuration, bool overlap)
{
    if (!IsWorldValid())
    {
        UE_LOG(LogMusicManager, Error, TEXT("Playing songs without a world is not possible!"));
        return;
    }

    if (song == nullptr)
    {
        UE_LOG(LogMusicManager, Error, TEXT("Unable to play song, song is nullptr!"));
        return;
    }

    if (BackgroundSong == nullptr)
    {
        BackgroundSong = song;
        AudioComponent = UGameplayStatics::CreateSound2D(World, song, 1.0f, 1.0f, 0.0f, nullptr, true, true);
        AudioComponent->FadeIn(fadeDuration);
    }
    else
    {
        AudioComponent->FadeOut(fadeDuration, 0.0f);
        StopDelayed(fadeDuration);

        if (overlap)
        {
            AudioComponent = UGameplayStatics::CreateSound2D(World, song, 1.0f, 1.0f, 0.0f, nullptr, true, true);
            AudioComponent->FadeIn(fadeDuration);
        }
        else
        {
            FTimerHandle handle;
            World->GetTimerManager().SetTimer(handle, [this, song, fadeDuration]()
                {
                    AudioComponent = UGameplayStatics::CreateSound2D(World, song, 1.0f, 1.0f, 0.0f, nullptr, true, true);
                    AudioComponent->FadeIn(fadeDuration);
                }, fadeDuration, false);
        }

        World->GetTimerManager().ClearTimer(Timer);
    }

    World->GetTimerManager().SetTimer(Timer, 0.001f, true);
}

void UMusicManager::ChimeInPlay(USoundCue* song, USoundCue* chime, float startTime)
{
    if (!IsWorldValid())
    {
        UE_LOG(LogMusicManager, Error, TEXT("Playing songs without a world is not possible!"));
        return;
    }

    if (song == nullptr || chime == nullptr)
    {
        UE_LOG(LogMusicManager, Error, TEXT("Unable to play song, song or chime is nullptr!"));
        return;
    }

    UAudioComponent* ChimeComponent = UGameplayStatics::CreateSound2D(World, chime, 1.0f, 1.0f, 0.0f, nullptr, true);
    ChimeComponent->Play();

    FTimerHandle handle;
    World->GetTimerManager().SetTimer(handle, [this, song, startTime]()
        {
            if (BackgroundSong == nullptr)
            {
                BackgroundSong = song;
            }
            else
            {
                Stop();
                BackgroundSong = song;
                World->GetTimerManager().ClearTimer(Timer);
            }

            PlaySong(song, startTime);
        }, chime->GetDuration(), false);
}

void UMusicManager::PlaySong(USoundCue* song, float startTime)
{
    if (song == nullptr)
    {
        UE_LOG(LogMusicManager, Error, TEXT("Unable to play song, song is nullptr!"));
        return;

    }

    AudioComponent = UGameplayStatics::CreateSound2D(World, song, 1.0f, 1.0f, 0.0f, nullptr, true, true);
    AudioComponent->Play(startTime);
}

void UMusicManager::Interrupt(USoundCue* song, bool keepPaused)
{
    if (IsSongValid())
    {
        TPair<USoundCue*, bool> interruption(song, keepPaused);
        Interruptions.Push(interruption);
        Pause();
        while (!Interruptions.IsEmpty())
        {
            PlayInterruption(Interruptions.Top());
            Interruptions.Pop();
        }
    }
    else
    {
        UE_LOG(LogMusicManager, Warning, TEXT("Unable to interrupt, no active background song to interrupt! Play one first."));
    }
}

void UMusicManager::PlayInterruption(TPair<USoundCue*, bool> interruption)
{
    if (!IsWorldValid())
    {
        UE_LOG(LogMusicManager, Error, TEXT("Interrupting songs without a world is not possible!"));
        return;
    }

    USoundCue* song = interruption.Key;
    bool keepPaused = interruption.Value;

    if (song == nullptr)
    {
        UE_LOG(LogMusicManager, Error, TEXT("Unable to interrupt song, song is nullptr!"));
        return;
    }

    UAudioComponent* InterruptAudioComponent = UGameplayStatics::CreateSound2D(World, song, 1.0f, 1.0f, 0.0f, nullptr, true, true);
    InterruptAudioComponent->Play();

    FTimerHandle handle;
    World->GetTimerManager().SetTimer(handle, [this, interruption, song, keepPaused]()
        {
            if (Interruptions.IsEmpty())
            {
                if (!keepPaused)
                {
                    Resume();
                }
            }

            SongInterrupted.Broadcast(song, keepPaused);
        }, song->GetDuration(), false);
}

void UMusicManager::ClearInterruptions()
{
    UE_LOG(LogMusicManager, Display, TEXT("Clearing all interruptions."));

    while (!Interruptions.IsEmpty())
    {
        Interruptions.Pop();
    }

    Resume();
}

void UMusicManager::Restart(float fadeOutTime)
{
    if (IsSongValid())
    {
        UE_LOG(LogMusicManager, Display, TEXT("Restarting background song %s"), *BackgroundSong->GetName());

        if (!fadeOutTime)
        {
            Stop();
            AudioComponent->Play();
        }
        else
        {
            AudioComponent->FadeOut(fadeOutTime, 0.0f);
            StopDelayed(fadeOutTime);

            FTimerHandle delay;

            World->GetTimerManager().SetTimer(delay, [this]() 
                {
                    AudioComponent->SetSound(BackgroundSong);
                    AudioComponent->Play();
                }, fadeOutTime, false);
        }
    }
}

void UMusicManager::Stop()
{
    if (IsSongValid())
    {
        AudioComponent->Stop();
        UE_LOG(LogMusicManager, Display, TEXT("Stopped active BackgroundSong"));
    }
}

void UMusicManager::StopDelayed(float delay)
{
    if (IsSongValid())
    {
        AudioComponent->StopDelayed(delay);
        UE_LOG(LogMusicManager, Display, TEXT("Stopped active BackgroundSong"));
    }
}

void UMusicManager::Pause()
{
    if (IsSongValid())
    {
        AudioComponent->SetPaused(true);
        UE_LOG(LogMusicManager, Display, TEXT("Paused active BackgroundSong"));
    }
}

void UMusicManager::Resume()
{
    if (IsSongValid())
    {
        AudioComponent->SetPaused(false);
        UE_LOG(LogMusicManager, Display, TEXT("Resumed active BackgroundSong"));
    }
}

void UMusicManager::SetVolume(float volume)
{
    if (IsSongValid())
    {
        AudioComponent->SetVolumeMultiplier(volume);
    }
}

void UMusicManager::SetPitch(float pitch)
{
    if (IsSongValid())
    {
        AudioComponent->SetPitchMultiplier(pitch);
    }
}

void UMusicManager::SetTime(float newTime)
{
    if (IsSongValid())
    {
        Play(BackgroundSong, newTime);
    }
}

bool UMusicManager::IsWorldValid()
{
    return (World = GetWorld()) != nullptr;
}

bool UMusicManager::IsSongValid() const
{
    return BackgroundSong != nullptr && AudioComponent != nullptr;
}

float UMusicManager::GetSongTime() const
{
    return World->GetTimerManager().GetTimerElapsed(Timer);
}

float UMusicManager::GetSongDuration() const
{
    if (IsSongValid())
    {
        return BackgroundSong->GetDuration();
    }
    
    return 0.0f;
}

USoundCue* UMusicManager::GetSong() const
{
    return BackgroundSong;
}
