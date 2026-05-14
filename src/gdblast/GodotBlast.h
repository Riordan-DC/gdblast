//© Copyright 2014-2022, Juan Linietsky, Ariel Manzur and the Godot community (CC-BY 3.0)
#ifndef BlastActor_CLASS_H
#define BlastActor_CLASS_H

// We don't need windows.h in this plugin but many others do and it throws up on itself all the time
// So best to include it and make sure CI warns us when we use something Microsoft took for their own goals....
#ifdef WIN32
#include <windows.h>
#endif

// Godot
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/rigid_body3d.hpp>
#include <godot_cpp/classes/encoded_object_as_id.hpp>

// nvBlast
#include "sdk/lowlevel/include/NvBlast.h"
#include "sdk/toolkit/include/NvBlastTk.h"
#include "sdk/extensions/shaders/include/NvBlastExtDamageShaders.h"

// Stress
#include "sdk/extensions/stress/include/NvBlastExtStressSolver.h"
#include "NvBlastArray.h"
#include "NvBlastHashSet.h"

#include "GDBlastSingleton.h"

using namespace Nv::Blast;
using namespace godot;

class BlastGroup;

// NvBlastActor -> RigidBody3D
// 

class BlastChunk : public RefCounted {
    GDCLASS(BlastChunk, RefCounted);

protected:
    static void _bind_methods() {
        ClassDB::bind_method(D_METHOD("get_position"), &BlastChunk::get_position);
        ClassDB::bind_method(D_METHOD("set_position", "position"), &BlastChunk::set_position);

        ClassDB::bind_method(D_METHOD("get_volume"), &BlastChunk::get_volume);
        ClassDB::bind_method(D_METHOD("set_volume", "volume"), &BlastChunk::set_volume);

        ClassDB::bind_method(D_METHOD("is_static"), &BlastChunk::is_static);
        
	    ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "position"), "set_position", "get_position");
        ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "volume"), "set_volume", "get_volume");
    }

public:
    BlastChunk() {}
    BlastChunk(NvBlastChunkDesc& chunk, uint32_t index, bool is_static) {
        position.x = chunk.centroid[0];
        position.y = chunk.centroid[1];
        position.z = chunk.centroid[2];
        volume = chunk.volume;
        parent_chunk = chunk.parentChunkIndex;
        chunk_index = index;
        is_support = (chunk.flags & NvBlastChunkDesc::SupportFlag) ? true : false;
        userdata = chunk.userData;
        is_chunk_static = is_static; 
    }

    ~BlastChunk() {
    }

    void set_position(Vector3 new_pos) {
        position = new_pos;
    }

    Vector3 get_position() {
        return position;
    }

    void set_volume(float vol) {
        volume = vol;
    }

    float get_volume() {
        return volume;
    }

    bool is_static() {
        return is_chunk_static;
    }

    Vector3 position;
    float volume;
    uint32_t parent_chunk;
    uint32_t chunk_index;
    bool is_support;
    uint32_t userdata;
    bool is_chunk_static;
};

class BlastActor;

class BlastListener : public TkEventListener {
public:
    BlastListener(BlastGroup* group);

    void receive(const TkEvent* events, uint32_t eventCount) override;

    BlastGroup* group;
};


//MAKE_TYPED_ARRAY(BlastChunk, Variant::OBJECT)
//MAKE_TYPED_ARRAY_INFO(BlastChunk, Variant::OBJECT)

class BlastFamily;

class BlastActor : public RefCounted {
    GDCLASS(BlastActor, RefCounted);

protected:
    static void _bind_methods();

public:
    BlastActor();
    BlastActor(TkActor* actor);
    ~BlastActor();

    void test();

    void add_bond(int index_a, int index_b, Vector3 centroid, Vector3 normal, float unit_area);
    BlastChunk* add_chunk(uint32_t parent_chunk, Vector3 centroid, float volume, bool is_support, uint32_t userdata, bool is_static);
    
    BlastFamily* create_actor();
    void apply_radial_damage(Vector3 position, float damage_in_health, float min_radius, float max_radius);
    void apply_spread_damage(Vector3 position, float damage_in_health, float min_distance, float max_distance);

    uint32_t get_visible_chunk_count();
    PackedInt32Array get_visible_chunk_indices();
    godot::Array get_chunks();



    void set_material(float health, float min_damage_threshold, float max_damage_threshold);

    void set_userdata(const uint64_t userdata);
    uint64_t get_userdata();
    PackedInt32Array get_joints();

    NvBlastExtMaterial* material = nullptr;
    TkActor* actor = nullptr;
    NvBlastExtProgramParams* params = nullptr;

    Variant userdata;

    PackedInt32Array chunk_indices;
    godot::Array blast_chunks;

    std::vector<NvBlastChunkDesc> chunks;
    std::vector<NvBlastBondDesc> bonds;

    RigidBody3D* body = nullptr;

    void set_body(RigidBody3D* p_body) {
        body = p_body;
    }

};

class BlastFamily : public RefCounted {
    GDCLASS(BlastFamily, RefCounted);

protected:
    static void _bind_methods() {
        ClassDB::bind_method(D_METHOD("stress_update"), &BlastFamily::stress_update);
        ClassDB::bind_method(D_METHOD("stress_actor_created"), &BlastFamily::stress_actor_created);
        ClassDB::bind_method(D_METHOD("stress_actor_destroyed"), &BlastFamily::stress_actor_destroyed);
    }

public:
    BlastFamily(){};
    BlastFamily(BlastActor* actor) : family(&actor->actor->getFamily()) {
        // Stress
        NvBlastFamily *familyLL = const_cast<NvBlastFamily *>(family->getFamilyLL());
        if (familyLL == NULL)
        {
            printf("Family is null\n");
            return;
        }
        Nv::Blast::ExtStressSolverSettings settings;
        settings.hardness = 0.01;
        settings.stressLinearFactor = 10.0;
        //settings.stressAngularFactor;
        //settings.bondIterationsPerFrame;
        //settings.graphReductionLevel;
        m_solver = ExtStressSolver::create(*familyLL, settings);
        m_solver->setAllNodesInfoFromLL();
        stress_actor_created(actor);
    }

    ~BlastFamily() {
        family->release(); // Do NOT call until all actors in family are gone!
    };
    
    TkFamily* family;

    // Stress
	ExtStressSolver* m_solver = nullptr;
	HashSet<BlastActor*>::type bodies;

    void stress_update(bool do_damage);

    void stress_actor_created(BlastActor *body)
    {
        TkActor *body_actor = body->actor;
        if (m_solver->notifyActorCreated(*body_actor->getActorLL()))
        {
            bodies.insert(body);
        }
    }

    void stress_actor_destroyed(BlastActor *body)
    {
        TkActor *body_actor = body->actor;
        m_solver->notifyActorDestroyed(*body_actor->getActorLL());
        bodies.erase(body);
    }

};

class BlastGroup : public RefCounted {
    GDCLASS(BlastGroup, RefCounted);

protected:
    static void _bind_methods() {
        ClassDB::bind_method(D_METHOD("add_actor", "actor"), &BlastGroup::add_actor);
        ClassDB::bind_method(D_METHOD("process"), &BlastGroup::process);
        ADD_SIGNAL(MethodInfo("on_actor_split", PropertyInfo(Variant::INT, "old_actor"), PropertyInfo(Variant::ARRAY, "new_actors")));
    }

public:
    BlastGroup();
    ~BlastGroup();
    void add_actor(BlastActor* actor);
    void process();
    void split_event(const TkSplitEvent* split_event);

    TkGroup* group = nullptr;
    BlastListener* listener = nullptr;
};


#endif // BlastActor_CLASS_H