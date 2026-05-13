//© Copyright 2014-2022, Juan Linietsky, Ariel Manzur and the Godot community (CC-BY 3.0)
#include "GodotBlast.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/engine.hpp>

using namespace godot;

bool myBondIsJointedFunction(uint32_t i) {
    return false;
}

// Blast Log function ptr
// typedef void(*NvBlastLog)(int type, const char* msg, const char* file, int line);
void logFn(int type, const char* msg, const char* file, int line) {
    printf(msg);
}

void BlastActor::add_bond(int index_a, int index_b, Vector3 centroid, Vector3 normal, float unit_area) {
    NvBlastBondDesc bond;
    bond.chunkIndices[0] = index_a;   // chunkIndices refer to chunk descriptor indices for support chunks
    bond.chunkIndices[1] = index_b;
    bond.bond.normal[0] = (float)normal.x; // normal in the +x direction
    bond.bond.normal[1] = (float)normal.y;
    bond.bond.normal[2] = (float)normal.z;
    bond.bond.area = unit_area;  // unit area
    bond.bond.centroid[0] = (float)centroid.x;   // centroid position in asset-local space
    bond.bond.centroid[1] = (float)centroid.y;
    bond.bond.centroid[2] = (float)centroid.z;
    bonds.push_back(bond);
}

BlastChunk* BlastActor::add_chunk(uint32_t parent_chunk, Vector3 centroid, float volume, bool is_support, uint32_t userdata, bool is_static) {
    NvBlastChunkDesc chunk;

    chunk.parentChunkIndex = parent_chunk;    // invalid index denotes a chunk hierarchy root
    chunk.centroid[0] = (float)centroid.x;   // centroid position in asset-local space
    chunk.centroid[1] = (float)centroid.y;
    chunk.centroid[2] = (float)centroid.z;
    chunk.volume = volume;    // Unit volume
    chunk.flags = is_support ? NvBlastChunkDesc::SupportFlag : NvBlastChunkDesc::NoFlags;
    chunk.userData = userdata; // User-supplied ID.  For example, this can be the index of the chunkDesc.
    chunks.push_back(chunk);

    uint32_t index = chunks.size() - 1;
    BlastChunk* bchunk = memnew(BlastChunk(chunk, index, is_static));
    return bchunk;
}

void BlastActor::test() {
    TkFramework* framework = NvBlastFramework::get_framework(); //blast_singleton->get_framework(); //NvBlastTkFrameworkCreate();

    std::vector<NvBlastChunkDesc> chunk_descs;
    size_t chunk_count = 3;
    chunk_descs.resize(chunk_count);
    chunk_descs[0].parentChunkIndex = UINT32_MAX;    // invalid index denotes a chunk hierarchy root
    chunk_descs[0].centroid[0] = 0.0f;   // centroid position in asset-local space
    chunk_descs[0].centroid[1] = 0.0f;
    chunk_descs[0].centroid[2] = 0.0f;
    chunk_descs[0].volume = 1.0f;    // Unit volume
    chunk_descs[0].flags = NvBlastChunkDesc::NoFlags;
    chunk_descs[0].userData = 0; // User-supplied ID.  For example, this can be the index of the chunkDesc.
                                // The userData can be left undefined.
    chunk_descs[1].parentChunkIndex = 0; // child of chunk described by chunkDescs[0]
    chunk_descs[1].centroid[0] = 0.0f;   // centroid position in asset-local space
    chunk_descs[1].centroid[1] = 1.0f;
    chunk_descs[1].centroid[2] = 0.0f;
    chunk_descs[1].volume = 1.0f;    // Unit volume
    chunk_descs[1].flags = NvBlastChunkDesc::SupportFlag; // This chunk should be represented in the support graph
    chunk_descs[2].parentChunkIndex = 0; // child of chunk described by chunkDescs[0]
    chunk_descs[2].centroid[0] = 0.0f;   // centroid position in asset-local space
    chunk_descs[2].centroid[1] = 2.0f;
    chunk_descs[2].centroid[2] = 0.0f;
    chunk_descs[2].volume = 1.0f;    // Unit volume
    chunk_descs[2].flags = NvBlastChunkDesc::SupportFlag; // This chunk should be represented in the support graph

    std::vector<NvBlastBondDesc> bond_descs;
    size_t bond_count = 1;
    bond_descs.resize(bond_count);
    bond_descs[0].chunkIndices[0] = 1;   // chunkIndices refer to chunk descriptor indices for support chunks
    bond_descs[0].chunkIndices[1] = 2;
    bond_descs[0].bond.normal[0] = 1.0f; // normal in the +x direction
    bond_descs[0].bond.normal[1] = 0.0f;
    bond_descs[0].bond.normal[2] = 0.0f;
    bond_descs[0].bond.area = 1.0f;  // unit area
    bond_descs[0].bond.centroid[0] = 0.0f;   // centroid position in asset-local space
    bond_descs[0].bond.centroid[1] = 0.0f;
    bond_descs[0].bond.centroid[2] = 0.0f;
    //bond_descs[0].userData = 0;  // this can be used to tell the user more information about this
                                    // bond for example to create a joint when this bond breaks
    TkAssetDesc asset_desc = {};
    asset_desc.chunkCount = chunk_count;
    asset_desc.chunkDescs = chunk_descs.data();
    asset_desc.bondCount = bond_count;
    asset_desc.bondDescs = bond_descs.data();

    std::vector<uint8_t> bondFlags(asset_desc.bondCount, 0); // Clear all flags
    for (uint32_t i = 0; i < asset_desc.bondCount; ++i)
    {
        if (myBondIsJointedFunction(i)) // User-authored
        {
            bondFlags[i] |= (uint8_t)TkAssetDesc::BondJointed;
        }
    }
    asset_desc.bondFlags = bondFlags.data();

    TkAsset* asset = framework->createAsset(asset_desc);  // Create a new TkAsset   
    UtilityFunctions::print("Asset created");

    TkActorDesc actor_desc = {};   // The TkActorDesc constructor sets sane default values for the base (NvBlastActorDesc) fields, giving uniform chunk and bond healths of 1.0.
    actor_desc.asset = asset; // This field of TkActorDesc must be set to a valid asset pointer.

    TkActor* test_actor = framework->createActor(actor_desc);
    BlastListener test_listener(nullptr);
    test_actor->getFamily().addListener(test_listener);

    UtilityFunctions::print("Listener added");

    TkGroupDesc groupDesc = {};
    groupDesc.workerCount = 1;  // this example processes groups on the calling thread only

    TkGroup* group1 = framework->createGroup(groupDesc);
    group1->addActor(*test_actor);

    UtilityFunctions::print("Added to group");

    NvBlastExtMaterial test_material = {};
    test_material.health = 0.1f;
    test_material.minDamageThreshold = 0.0f;
    test_material.maxDamageThreshold = 0.0f; // 100% of damage can be applied

    NvBlastDamageProgram program = {NvBlastExtShearGraphShader, NvBlastExtShearSubgraphShader};

    NvBlastExtRadialDamageDesc damageDesc = {};
    //damageDesc.compressive = 10.0f;
    damageDesc.damage = 2.0; // completely destroy
    damageDesc.position[0] = 0.0f;
    damageDesc.position[1] = 0.0f;
    damageDesc.position[2] = 0.0f;
    damageDesc.minRadius = 0.0f;
    damageDesc.maxRadius = 100.0f;

    NvBlastExtProgramParams params = {&damageDesc, &test_material, nullptr};

    test_actor->damage(program, &params);

    UtilityFunctions::print("Actor damaged");

    group1->process();
    group1->release();
    test_actor->release();
}

void BlastActor::create_actor() {
    ERR_FAIL_COND(chunks.empty());
    ERR_FAIL_COND(bonds.empty()); // BUG: This needs to let the user know something wrong happened

    TkFramework* framework = NvBlastFramework::get_framework();

    TkAssetDesc asset_desc = {};
    asset_desc.chunkCount = (uint32_t)chunks.size();
    asset_desc.chunkDescs = chunks.data();
    asset_desc.bondCount = (uint32_t)bonds.size();
    asset_desc.bondDescs = bonds.data();

    TkAsset* asset = framework->createAsset(asset_desc);   
    TkActorDesc actor_desc = {};   // The TkActorDesc constructor sets sane default values for the base (NvBlastActorDesc) fields, giving uniform chunk and bond healths of 1.0.
    actor_desc.asset = asset; // This field of TkActorDesc must be set to a valid asset pointer.
    actor = framework->createActor(actor_desc);
}

void BlastActor::apply_radial_damage(Vector3 position, float damage, float min_radius, float max_radius) {
    ERR_FAIL_COND(actor == nullptr);
    ERR_FAIL_COND(material == nullptr);

    // Damage programs
    NvBlastDamageProgram program = { NvBlastExtFalloffGraphShader, NvBlastExtFalloffSubgraphShader };

    // Memory leaky, alloc once and store
    NvBlastExtRadialDamageDesc* damageDesc = new NvBlastExtRadialDamageDesc();
    damageDesc->damage = damage;
    damageDesc->position[0] = (float)position.x;
    damageDesc->position[1] = (float)position.y;
    damageDesc->position[2] = (float)position.z;
    damageDesc->minRadius = min_radius;
    damageDesc->maxRadius = max_radius;

    params->damageDesc = damageDesc;
    params->material = material;
    params->accelerator = nullptr;

    // Apply damage
    actor->damage(program, params);
}


void BlastActor::apply_spread_damage(Vector3 position, float damage, float min_distance, float max_distance) {
    ERR_FAIL_COND(actor == nullptr);
    ERR_FAIL_COND(material == nullptr);

    // Damage programs
    NvBlastDamageProgram program = { NvBlastExtImpactSpreadGraphShader, NvBlastExtImpactSpreadSubgraphShader };

    // Memory leaky, alloc once and store
    NvBlastExtImpactSpreadDamageDesc* damageDesc = new NvBlastExtImpactSpreadDamageDesc();
    damageDesc->damage = damage;
    damageDesc->position[0] = (float)position.x;
    damageDesc->position[1] = (float)position.y;
    damageDesc->position[2] = (float)position.z;
    damageDesc->minRadius = min_distance;
    damageDesc->maxRadius = max_distance;

    params->damageDesc = damageDesc;
    params->material = material;
    params->accelerator = nullptr;

    // Apply damage
    actor->damage(program, params);
}





Array BlastActor::get_chunks() {
    return blast_chunks; // this is useless and will be removed
}

uint32_t BlastActor::get_visible_chunk_count() {
    return actor->getVisibleChunkCount();
    //return NvBlastActorGetVisibleChunkCount(actor->getActorLL(), logFn);
}

PackedInt32Array BlastActor::get_visible_chunk_indices() {
    uint32_t chunk_count = get_visible_chunk_count();
    chunk_indices.resize(chunk_count);
    actor->getVisibleChunkIndices((uint32_t*)chunk_indices.ptr(), chunk_count); //, actor->getActorLL(), logFn);
    return chunk_indices;
}

void BlastActor::set_material(float health, float min_damage_threshold, float max_damage_threshold) {
    material->health = health;
    material->minDamageThreshold = min_damage_threshold;
    material->maxDamageThreshold = max_damage_threshold;
}

BlastActor::BlastActor(TkActor* actor) {
    this->actor = actor;
    material = new NvBlastExtMaterial();
    params = (NvBlastExtProgramParams*)malloc(sizeof(NvBlastExtProgramParams));
}

BlastActor::BlastActor() {
    // The example material "Material" is modeled after NvBlastExtMaterial in the NvBlastExtShaders extension
    material = new NvBlastExtMaterial();
    actor = nullptr;
    params = (NvBlastExtProgramParams*)malloc(sizeof(NvBlastExtProgramParams));
}

BlastActor::~BlastActor() {
    /*
    if (actor != nullptr) {
        actor->release();
    }

    if (material != nullptr) {
        delete material;
    }

    if (params != nullptr) {
        delete params;
    }
    */
}

void BlastActor::set_userdata(const uint64_t userdata) {
    //this->userdata = userdata;
    actor->userData = (void*)userdata;
}

uint64_t BlastActor::get_userdata() {
    //return userdata;
    return (uint64_t)actor->userData;
}

PackedInt32Array BlastActor::get_joints() {
    //return userdata;
    PackedInt32Array ret;
    const NvBlastSupportGraph graph = actor->getAsset()->getGraph();

    uint32_t* adjacencyPartition;

    int64_t bond_count = actor->getAsset()->getJointDescCount();
    const TkAssetJointDesc* joints = actor->getAsset()->getJointDescs();
    ret.resize(bond_count * 2);

    for (int i = 0; i < bond_count; i+=2) {
        ret[i] = joints[i].nodeIndices[0];
        ret[i+1] = joints[i].nodeIndices[1];
    }
    return ret;
}

void BlastActor::_bind_methods() {
    ClassDB::bind_method(D_METHOD("test"), &BlastActor::test);
    
    ClassDB::bind_method(D_METHOD("create_actor"), &BlastActor::create_actor);
    ClassDB::bind_method(D_METHOD("set_userdata", "userdata"), &BlastActor::set_userdata);
    ClassDB::bind_method(D_METHOD("get_userdata"), &BlastActor::get_userdata);
    ClassDB::bind_method(D_METHOD("get_joints"), &BlastActor::get_joints);

    ClassDB::bind_method(D_METHOD("apply_radial_damage", "position", "damage", "min_radius", "max_radius"), &BlastActor::apply_radial_damage);
    ClassDB::bind_method(D_METHOD("apply_spread_damage", "position", "damage", "min_distance", "max_distance"), &BlastActor::apply_radial_damage);
    
    ClassDB::bind_method(D_METHOD("add_chunk", "parent_chunk", "centroid", "volume", "is_support", "userdata", "is_static"), &BlastActor::add_chunk);
    ClassDB::bind_method(D_METHOD("add_bond", "index_a", "index_b", "centroid", "normal", "unit_area"), &BlastActor::add_bond);

    ClassDB::bind_method(D_METHOD("get_visible_chunk_count"), &BlastActor::get_visible_chunk_count);
    ClassDB::bind_method(D_METHOD("get_visible_chunk_indices"), &BlastActor::get_visible_chunk_indices);
    ClassDB::bind_method(D_METHOD("get_chunks"), &BlastActor::get_chunks);

    ClassDB::bind_method(D_METHOD("set_material", "health", "min_damage_threshold", "max_damage_threshold"), &BlastActor::set_material);
    
    

}


///////////////////////////////////////////
// Listener
///////////////////////////////////////////

BlastListener::BlastListener(BlastGroup* group) {
    this->group = group;
}

// TkEventListener interface
void BlastListener::receive(const TkEvent* events, uint32_t eventCount) {
    
    // Events are batched into an event buffer.  Loop over all events:
    for (uint32_t i = 0; i < eventCount; ++i) {
        const TkEvent& event = events[i];
        // See TkEvent documentation for event types
        switch (event.type) {
        case TkSplitEvent::EVENT_TYPE: { // A TkActor has split into smaller actors
            if (group != nullptr) {
                const TkSplitEvent* splitEvent = event.getPayload<TkSplitEvent>();  // Split event payload
                group->split_event(splitEvent);
            }
        }
        break;
        case TkJointUpdateEvent::EVENT_TYPE: {
            const TkJointUpdateEvent* jointEvent = event.getPayload<TkJointUpdateEvent>();  // Joint update event payload
            // Joint events have three subtypes, see which one we have
            switch (jointEvent->subtype)
            {
            case TkJointUpdateEvent::External:
                //myCreateJointFunction(jointEvent->joint);   // An internal joint has been "exposed" (now joins two different actors).  Create a physics joint.
                break;
            case TkJointUpdateEvent::Changed:
                //myUpdatejointFunction(jointEvent->joint);   // A joint's actors have changed, so we need to update its corresponding physics joint.
                break;
            case TkJointUpdateEvent::Unreferenced:
                //myDestroyJointFunction(jointEvent->joint);  // This joint is no longer referenced, so we may delete the corresponding physics joint.
                break;
            }
        }

        // Unhandled:
        case TkFractureCommands::EVENT_TYPE: {
            break;
        }

        case TkFractureEvents::EVENT_TYPE:
        default:
        break;
        }
    }
}


//////////////////////////////////
// Blast group

BlastGroup::BlastGroup() {
    TkFramework* framework = NvBlastFramework::get_framework();
    TkGroupDesc groupDesc = {};
    groupDesc.workerCount = 1;  // this example processes groups on the calling thread only
    group = framework->createGroup(groupDesc);
    listener = new BlastListener(this);
}

BlastGroup::~BlastGroup() {
    if (group != nullptr) {
        group->release();
    }
}

void BlastGroup::add_actor(BlastActor* actor) {
    ERR_FAIL_COND(group == nullptr);
    group->addActor(*actor->actor);
    actor->actor->getFamily().addListener(*listener);
}

void BlastGroup::process() {
    ERR_FAIL_COND(group == nullptr);
    group->process();
}

void BlastGroup::split_event(const TkSplitEvent* split_event) {
    // Instead of doing this slow bullshit we should just call a script callback
    Array new_actors;
    for (uint32_t j = 0; j < split_event->numChildren; ++j) {
        BlastActor* new_actor = memnew(BlastActor(split_event->children[j]));
        new_actors.append(new_actor);
    } 

    emit_signal("on_actor_split", (uint64_t)split_event->parentData.userData, new_actors);

    // The parent actor may no longer be valid.  Instead, we receive the information it held
    // which we need to update our app's representation (e.g. removal of the corresponding physics actor)
    //split_remove_actor(split_event->parentData.family, split_event->parentData.index, split_event->parentData.userData);
    // TODO: Emit remove_actor signal with this actor

    // The split event contains an array of "child" actors that came from the parent.  These are valid
    // TkActor pointers and may be used to create physics and graphics representations in our application
    /*
    for (uint32_t j = 0; j < split_event->numChildren; ++j)
    {
        split_create_actor(split_event->children[j]);
    }
    */
}