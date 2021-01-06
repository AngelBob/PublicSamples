#pragma once

class InGameObject;
enum class ObjectType;

namespace ResourceLoader
{
    static int32_t globalId;

    __inline void LoadStringResource( json& items, const wchar_t *resourceName, int32_t resourceId )
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
    __inline void BuildObjects( const json &objJson, ObjectType type, std::vector<std::shared_ptr<InGameObject>>& list )
    {
        // Iterate through the array of objects; adding each to the provided list.

        // Object IDs are 1-based, but the vector is 0-based.  To enable accessing
        // the vector via the ObjectID, insert an invalid object as the first
        // item in the vector.
        if( 0 == list.size() )
        {
            globalId = 0;
            list.reserve( objJson.size() + 1 );

            json badObj = "{ \"Id\": 0, \"Name\": \"invalid\", \"Responses\": [ { \"Type\": \"Examine\", \"Text\": \"invalid object\" } ] }"_json;
            list.emplace_back( std::make_shared<GameObjectT>( badObj, type, 0, 0 ) );
        }

        int32_t classId = 0;
        for( auto it = objJson.begin(); it != objJson.end(); ++it )
        {
            auto& item = list.emplace_back( std::make_shared<GameObjectT>( *it, type, ++classId, ++globalId ) );
        }
    }

}