#ifndef __ONVIF_DEVICE_PLUGIN_CONFIGURE_H__
#define __ONVIF_DEVICE_PLUGIN_CONFIGURE_H__

#include <DeviceLayer/DeviceLayerDef.h>
#include <DeviceLayer/ConfigTargets.h>

using namespace DeviceLayer;

#define MAKERESO(w,h)   (#w "x" #h)

enum ONVIF_ACTION_TYPE
{
    ONVIF_ACTION_NONE,
    ONVIF_ACTION_GET_VIDEO,
    ONVIF_ACTION_GET_OSD,
    ONVIF_ACTION_SET_VIDEO,
    ONVIF_ACTION_SET_OSD,
    ONVIF_ACTION_SET_NTP,
    ONVIF_ACTION_INVOKE,
};

#define CONFIG_PARAMS_GETSETTING_INVALID -1
#define CONFIG_PARAMS_SETSETTING_INVALID -2

struct OnvifMediaResolution
{
    OnvifMediaResolution& operator = (const OnvifMediaResolution& other)
    {
        if(this != &other) {
            if(other.isGetValid()) {
                width = other.width;
                height = other.height;
            }
        }
        return *this;
    }
    bool isSetValid()
    {
        return (width != CONFIG_PARAMS_SETSETTING_INVALID
                && height != CONFIG_PARAMS_SETSETTING_INVALID);
    }

    bool isGetValid() const
    {
        return (width != CONFIG_PARAMS_GETSETTING_INVALID
                && height != CONFIG_PARAMS_GETSETTING_INVALID);
    }

    int width;
    int height;
};

struct OnvifMediaArguments
{
    int bitrate;
    int framerate;
    OnvifMediaResolution resolution;
};

struct OnvifImageArguments
{
    bool operator == (const OnvifImageArguments& other) const
    {
        return (brightness == other.brightness
                && contrast == other.contrast
                && saturation == other.saturation);
    }
    int brightness;
    int contrast;
    int saturation;
};

struct OnvifStreamArguments
{
    OnvifMediaArguments mediaArg;
    OnvifImageArguments imageArg;
};

struct OnvifVideoArguments
{
    OnvifVideoArguments(CONFIG_ACTION_TYPE actiontype)
        : actionType(actiontype)
    {
        if (actiontype == CFG_ACTION_GET) {
            mainStream.mediaArg.bitrate = CONFIG_PARAMS_GETSETTING_INVALID;
            mainStream.mediaArg.framerate = CONFIG_PARAMS_GETSETTING_INVALID;
            mainStream.mediaArg.resolution.height = CONFIG_PARAMS_GETSETTING_INVALID;
            mainStream.mediaArg.resolution.width = CONFIG_PARAMS_GETSETTING_INVALID;
            mainStream.imageArg.brightness = CONFIG_PARAMS_GETSETTING_INVALID;
            mainStream.imageArg.contrast = CONFIG_PARAMS_GETSETTING_INVALID;
            mainStream.imageArg.saturation = CONFIG_PARAMS_GETSETTING_INVALID;

            subStream.mediaArg.bitrate = CONFIG_PARAMS_GETSETTING_INVALID;
            subStream.mediaArg.framerate = CONFIG_PARAMS_GETSETTING_INVALID;
            subStream.mediaArg.resolution.height = CONFIG_PARAMS_GETSETTING_INVALID;
            subStream.mediaArg.resolution.width = CONFIG_PARAMS_GETSETTING_INVALID;
            subStream.imageArg.brightness = CONFIG_PARAMS_GETSETTING_INVALID;
            subStream.imageArg.contrast = CONFIG_PARAMS_GETSETTING_INVALID;
            subStream.imageArg.saturation = CONFIG_PARAMS_GETSETTING_INVALID;
        } else if (actiontype == CFG_ACTION_SET) {
            mainStream.mediaArg.bitrate = CONFIG_PARAMS_SETSETTING_INVALID;
            mainStream.mediaArg.framerate = CONFIG_PARAMS_SETSETTING_INVALID;
            mainStream.mediaArg.resolution.height = CONFIG_PARAMS_SETSETTING_INVALID;
            mainStream.mediaArg.resolution.width = CONFIG_PARAMS_SETSETTING_INVALID;
            mainStream.imageArg.brightness = CONFIG_PARAMS_SETSETTING_INVALID;
            mainStream.imageArg.contrast = CONFIG_PARAMS_SETSETTING_INVALID;
            mainStream.imageArg.saturation = CONFIG_PARAMS_SETSETTING_INVALID;

            subStream.mediaArg.bitrate = CONFIG_PARAMS_SETSETTING_INVALID;
            subStream.mediaArg.framerate = CONFIG_PARAMS_SETSETTING_INVALID;
            subStream.mediaArg.resolution.height = CONFIG_PARAMS_SETSETTING_INVALID;
            subStream.mediaArg.resolution.width = CONFIG_PARAMS_SETSETTING_INVALID;
            subStream.imageArg.brightness = CONFIG_PARAMS_SETSETTING_INVALID;
            subStream.imageArg.contrast = CONFIG_PARAMS_SETSETTING_INVALID;
            subStream.imageArg.saturation = CONFIG_PARAMS_SETSETTING_INVALID;
        }
    }

    const CONFIG_ACTION_TYPE actionType;
    OnvifStreamArguments mainStream;
    OnvifStreamArguments subStream;
};

enum LAST_OSD_CONDITION{
    LAST_CONDITION_NONE,
    LAST_CONDITION_SET,
    LAST_CONDITION_CREATE,
    LAST_CONDITION_DELETE
};

#define OSD_NOT_EXISTENT -2
struct OnvifOSDPosition
{
    OnvifOSDPosition()
        :x(new float(sizeof(float)))
        ,y(new float(sizeof(float)))
    {
        *x = OSD_NOT_EXISTENT;
        *y = OSD_NOT_EXISTENT;
    }

    float *x;
    float *y;
};

struct OnvifOSDArguments
{
    OnvifOSDArguments()
        : datetimePos(new OnvifOSDPosition)
        , customPos(new OnvifOSDPosition)
    {
        ResetDateTimeOSD();
        ResetCustomTextOSD();
    }
    ~OnvifOSDArguments()
    {

    }
    void ResetDateTimeOSD()
    {
        bShowDateTime = false;
        setDateTimeCondition = LAST_CONDITION_NONE;
        *datetimePos->x = OSD_NOT_EXISTENT;
        *datetimePos->y = OSD_NOT_EXISTENT;
        datetimeToken = NULL;
    }

    void ResetCustomTextOSD()
    {
        bShowCustomText = false;
        setCustomTextCondition = LAST_CONDITION_NONE;
        *customPos->x = OSD_NOT_EXISTENT;
        *customPos->y = OSD_NOT_EXISTENT;
        customToken = NULL;
        memset(customText, 0, 90);
    }

    LAST_OSD_CONDITION setDateTimeCondition;
    LAST_OSD_CONDITION setCustomTextCondition;
    bool bShowDateTime;
    bool bShowCustomText;

    OnvifOSDPosition *datetimePos;
    OnvifOSDPosition *customPos;

    char customText[90];
    char *customToken;
    char *datetimeToken;
};

struct OnvifOSDsArguments
{
    OnvifOSDsArguments()
        : mainOSDArg(new OnvifOSDArguments)
        , subOSDArg(new OnvifOSDArguments)
    {

    }
    OnvifOSDArguments *mainOSDArg;
    OnvifOSDArguments *subOSDArg;
};
#endif
