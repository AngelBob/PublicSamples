#pragma once

namespace ResourceLoader
{
    __inline void LoadStringResource( json& items, const wchar_t *resourceName, int32_t resourceId )
    {
        HRSRC resFinder = ::FindResource( NULL, resourceName, MAKEINTRESOURCE( resourceId ) );
        if( NULL == resFinder )
        {
            return;
        }

        HGLOBAL resLoader = ::LoadResource( NULL, resFinder );
        if( NULL == resLoader )
        {
            return;
        }

        PVOID resource = ::LockResource( resLoader );
        if( NULL == resource )
        {
            return;
        }

        // Convert the raw string into json
        items = json::parse( static_cast<char*>( resource ) );
    }

    template<typename GameObjectT>
    __inline void BuildObjects( const json &objJson, std::vector<std::unique_ptr<GameObjectT>>& list )
    {
        // Iterate through the array of objects; adding each to the appropriate list.
        list.clear();

        // Object IDs are 1-based, but the vector is 0-based.  To enable accessing
        // the vector via the ObjectID, insert an invalid object as the first
        // item in the vector.
        list.reserve( objJson.size() + 1 );

        json badObj = "{ \"Id\": 0, \"Name\": \"invalid\" }"_json;
        list.emplace_back( std::make_unique<GameObjectT>( badObj ) );

        int32_t counter = 1;
        for( auto it = objJson.begin(); it != objJson.end(); ++it )
        {
            auto& item = list.emplace_back( std::make_unique<GameObjectT>( *it ) );

            // validate the objectID == array index strategy
            assert( item->GetObjectId() == counter );
            ++counter;
        }
    }

}