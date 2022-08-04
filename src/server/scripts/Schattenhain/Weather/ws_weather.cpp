/*
 * Schattenhain 2022
 */

#include "ScriptPCH.h"
#include "ScriptMgr.h"
#include "Log.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "MiscPackets.h"
#include "WorldSession.h"
#include "Player.h"
#include "Language.h"
#include "RBAC.h"
#include "Chat.h"
#include <HttpClient.h>
#include <Json.h>
#include <unordered_map>

class ws_weather : public WorldScript
{
private:
    enum WorldScriptWeather
    {
        WEATHER_EVENT_TICK = 1
    };

    struct GameWeatherXRealWeather
    {
        uint32 id;
        uint8 category;
        uint32 gameWeatherId = 0;
        float gameWeatherGrad = 0;
        bool defaultWeatherForCategory = false;
    };

    struct RealWeather
    {
        uint32 id;
        uint8 category;
        std::string icon = "01d";
        std::string description = "";
        std::string sourceCity = "";
        std::string soruceCountry = "";

        float temprature = 0.f;
        float feelTemprature = 0.f;
        int pressure = 0;
        int humidity = 0;
        float windSpeed = 0.f;
    };

    const static inline const char* API_HOST = "api.openweathermap.org";
    const static inline const char* API_PATH = "/data/2.5/weather?id=%u&units=metric&lang=de&appid=%s";
public:

    ws_weather() : WorldScript("ws_weather")
    {
        LoadFromDB();

        if (_gameWeatherXRealWeatherStore.size() == 0)
            return;

        LoadRealWeatherFromApi();

        _events.ScheduleEvent(WEATHER_EVENT_TICK, _interval);
    }

    void OnConfigLoad(bool reload) override
    {
        _active = sConfigMgr->GetBoolDefault("Schattenhain.Weather.Enable", false);
        _interval = std::chrono::seconds(sConfigMgr->GetIntDefault("Schattenhain.Weather.Interval", 300));
        _apiKey = sConfigMgr->GetStringDefault("Schattenhain.Weather.ApiKey", "").c_str();
        _apiCityCode = sConfigMgr->GetIntDefault("Schattenhain.Weather.CityId", 734517);

        LoadRealWeatherFromApi();
    }

    void OnUpdate(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
            case WEATHER_EVENT_TICK:

                if (_active)
                {
                    LoadRealWeatherFromApi();
                    UpdateWeatherForAllPlayer();
                }

                _events.ScheduleEvent(WEATHER_EVENT_TICK, _interval);
                break;
            }
        }
    }

    void SetWeatherForPlayer(Player* player)
    {
        if (_activeGameWeather == nullptr)
            return;

        uint32 weatherId = _activeGameWeather->gameWeatherId;
        float grad = _activeGameWeather->gameWeatherGrad;

        if (grad >= 1)
            grad = 0.99f;

        if (grad <= 0)
            grad = 0;

        WorldPackets::Misc::Weather weatherPkg(WeatherState(weatherId), grad, false);
        player->SendDirectMessage(weatherPkg.Write());
    }
private:
    void LoadRealWeatherFromApi()
    {
        if (_apiCityCode == 0 || _apiKey == "")
            return;

        try
        {
            std::string responseBody = Trinity::HttpClient::Get(API_HOST, Trinity::StringFormat(API_PATH, _apiCityCode, _apiKey).c_str());
            if (responseBody == "")
                return;

            JSON weatherData = JSON::Load(responseBody);
            if (!weatherData.hasKey("weather") || !weatherData["weather"][0].hasKey("id"))
                return;

            RealWeather realWeather;
            realWeather.id = weatherData["weather"][0]["id"].ToInt();
            realWeather.category = (uint8)std::to_string(realWeather.id).c_str()[0];

            if (weatherData.hasKey("name"))
                realWeather.sourceCity = weatherData["name"].ToString();

            if (weatherData.hasKey("sys") && weatherData["sys"].hasKey("country"))
                realWeather.soruceCountry = weatherData["sys"]["country"].ToString();
            
            if (weatherData["weather"][0].hasKey("icon"))
                realWeather.icon = weatherData["weather"][0]["icon"].ToString();

            if (weatherData["weather"][0].hasKey("description"))
                realWeather.description = weatherData["weather"][0]["description"].ToString();

            if (weatherData.hasKey("main"))
            {
                if (weatherData["main"].hasKey("temp"))
                    realWeather.temprature = weatherData["main"]["temp"].ToFloat();

                if (weatherData["main"].hasKey("feels_like"))
                    realWeather.feelTemprature = weatherData["main"]["feels_like"].ToFloat();

                if (weatherData["main"].hasKey("pressure"))
                    realWeather.pressure = weatherData["main"]["pressure"].ToInt();

                if (weatherData["main"].hasKey("humidity"))
                    realWeather.humidity = weatherData["main"]["humidity"].ToInt();
            }

            if (weatherData.hasKey("wind") && weatherData["wind"].hasKey("speed"))
                realWeather.windSpeed = weatherData["wind"]["speed"].ToFloat();

            _realWeather = realWeather;

            SetActiveGameWeather();
        }
        catch (std::exception const& e)
        {
            return;
        }
    }

    void LoadFromDB()
    {
        if (QueryResult result = WorldDatabase.Query("SELECT `id`, `category`, `game_weather_id`, `game_weather_grad`, `default_weather_for_category` FROM `game_weather_x_real_weather`"))
        {
            do
            {
                Field* fields = result->Fetch();

                GameWeatherXRealWeather* gameWeatherXRealWeather = new GameWeatherXRealWeather();
                gameWeatherXRealWeather->id = fields[0].GetUInt32();
                gameWeatherXRealWeather->category = fields[1].GetUInt8();
                gameWeatherXRealWeather->gameWeatherId = fields[2].GetUInt32();
                gameWeatherXRealWeather->gameWeatherGrad = fields[3].GetFloat();
                gameWeatherXRealWeather->defaultWeatherForCategory = fields[4].GetBool();

                _gameWeatherXRealWeatherStore[gameWeatherXRealWeather->id] = gameWeatherXRealWeather;
                _gameWeatherXRealWeatherDefaultCategoryStore[gameWeatherXRealWeather->category] = gameWeatherXRealWeather;
            } while (result->NextRow());
        }
    }

    void SetActiveGameWeather()
    {
        if (_gameWeatherXRealWeatherStore.find(_realWeather.id) != _gameWeatherXRealWeatherStore.end())
        {
            _activeGameWeather = _gameWeatherXRealWeatherStore[_realWeather.id];

            return;
        }

        if (_gameWeatherXRealWeatherDefaultCategoryStore.find(_realWeather.category) != _gameWeatherXRealWeatherDefaultCategoryStore.end())
        {
            _activeGameWeather = _gameWeatherXRealWeatherDefaultCategoryStore[_realWeather.category];

            return;
        }
    }

    void UpdateWeatherForAllPlayer()
    {
        HashMapHolder<Player>::MapType const& players = ObjectAccessor::GetPlayers();

        for (HashMapHolder<Player>::MapType::const_iterator itr = players.begin(); itr != players.end(); ++itr)
        {
            Player* player = itr->second;

            if (player && player->IsInWorld())
                SetWeatherForPlayer(player);
        }
    }

    EventMap _events;
    RealWeather _realWeather;
    const GameWeatherXRealWeather* _activeGameWeather = nullptr;
    std::unordered_map<uint32, const GameWeatherXRealWeather*> _gameWeatherXRealWeatherStore;
    std::unordered_map<uint8, const GameWeatherXRealWeather*> _gameWeatherXRealWeatherDefaultCategoryStore;

    // Config
    bool _active = false;
    std::chrono::seconds _interval = 300s;
    const char* _apiKey = "";
    uint32 _apiCityCode = 734517;
};

class ps_weather : public PlayerScript
{
public:
    ps_weather(ws_weather* worldScriptWeather) : PlayerScript("ps_weather")
    {
        _worldScriptWeather = worldScriptWeather;
    }

    void OnLogin(Player* player, bool /*firstLogin*/) override
    {
        _worldScriptWeather->SetWeatherForPlayer(player);
    }

    void OnMapChanged(Player* player) override
    {
        _worldScriptWeather->SetWeatherForPlayer(player);
    }

private:
    ws_weather* _worldScriptWeather;
};

void AddSC_Weather_WorldScript()
{
    ws_weather* worldScriptWeather = new ws_weather();
    new ps_weather(worldScriptWeather);
}

