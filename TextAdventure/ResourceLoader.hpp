#pragma once

class InGameObject;
class InGameEvent;

namespace ResourceLoader
{
    static int32_t globalId;

    inline void LoadStringResource( json& items, const wchar_t *resourceName, int32_t resourceId )
    {
        HRSRC resFind = ::FindResource( NULL, resourceName, MAKEINTRESOURCE( resourceId ) );
        if( NULL == resFind )
        {
            return;
        }

        HGLOBAL resLoad = ::LoadResource( NULL, resFind );
        if( NULL == resLoad )
        {
            return;
        }

        PVOID resource = ::LockResource( resLoad );
        if( NULL == resource )
        {
            return;
        }

        DWORD len = ::SizeofResource( NULL, resFind );

        // Convert the raw string into json
        std::string jsonIn( static_cast<char*>( resource ), len );
        items = json::parse( jsonIn );
    }

    template<typename GameObjectT>
    inline void BuildObjects( const json &objJson, std::vector<std::shared_ptr<InGameObject>>& list )
    {
        if( 0 == list.size() )
        {
            // reset the global id - each vector is accessed as an array based on global id
            // so if the vector is empty, the array index (e.g., globalId) needs to be
            // reset to zero to keep the ID and array index in sync.
            globalId = 0;
        }

        // Iterate through the array of objects; adding each to the provided list.
        for( auto it = objJson.begin(); it != objJson.end(); ++it )
        {
            list.emplace_back( std::static_pointer_cast<InGameObject>( std::make_shared<GameObjectT>( *it, globalId++ ) ) );
        }
    }

    inline void BuildEvents( const json &objJson, std::vector<std::shared_ptr<InGameEvent>>& list )
    {
        if( 0 == list.size() )
        {
            // reset the global id - each vector is accessed as an array based on global id
            // so if the vector is empty, the array index (e.g., globalId) needs to be
            // reset to zero to keep the ID and array index in sync.
            globalId = 0;
        }

        // Iterate through the array of objects; adding each to the provided list.
        for( auto it = objJson.begin(); it != objJson.end(); ++it )
        {
            list.emplace_back( std::make_shared<InGameEvent>( *it, globalId++ ) );
        }
    }

}