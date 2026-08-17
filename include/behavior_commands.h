#ifndef BEHAVIOR_COMMANDS_H
#define BEHAVIOR_COMMANDS_H

#define BC_B(a) _SHIFTL(a, 24, 8)
#define BC_BB(a, b) (_SHIFTL(a, 24, 8) | _SHIFTL(b, 16, 8))
#define BC_BBBB(a, b, c, d) (_SHIFTL(a, 24, 8) | _SHIFTL(b, 16, 8) | _SHIFTL(c, 8, 8) | _SHIFTL(d, 0, 8))
#define BC_BBH(a, b, c) (_SHIFTL(a, 24, 8) | _SHIFTL(b, 16, 8) | _SHIFTL(c, 0, 16))
#define BC_B0H(a, b) (_SHIFTL(a, 24, 8) | _SHIFTL(b, 0, 16))
#define BC_H(a) _SHIFTL(a, 16, 16)
#define BC_HH(a, b) (_SHIFTL(a, 16, 16) | _SHIFTL(b, 0, 16))
#define BC_W(a) ((uintptr_t)(u32)(a))
#define BC_PTR(a) ((uintptr_t)(a))

// Defines the start of the behavior script as well as the object list the object belongs to.
// Has some special behavior for certain objects.
#define BEGIN(objList) \
    BC_BB(0x00, objList)

// Delays the behavior script for a certain number of frames.
#define DELAY(num) \
    BC_B0H(0x01, num)

// Jumps to a new behavior command and stores the return address in the object's stack.
#define CALL(addr) \
    BC_B(0x02), \
    BC_PTR(addr)

// Jumps back to the behavior command stored in the object's stack.
#define RETURN() \
    BC_B(0x03)

// Jumps to a new behavior script without saving anything.
#define GOTO(addr) \
    BC_B(0x04), \
    BC_PTR(addr)

// Marks the start of a loop that will repeat a certain number of times.
#define BEGIN_REPEAT(count) \
    BC_B0H(0x05, count)

// Marks the end of a repeating loop.
#define END_REPEAT() \
    BC_B(0x06)

// Also marks the end of a repeating loop, but continues executing commands following the loop on the same frame.
#define END_REPEAT_CONTINUE() \
    BC_B(0x07)

// Marks the beginning of an infinite loop.
#define BEGIN_LOOP() \
    BC_B(0x08)

// Marks the end of an infinite loop.
#define END_LOOP() \
    BC_B(0x09)

// Exits the behavior script.
// Often used to end behavior scripts that do not contain an infinite loop.
#define BREAK() \
    BC_B(0x0A)

// Exits the behavior script, unused.
#define BREAK_UNUSED() \
    BC_B(0x0B)

// Executes a native game function.
#define CALL_NATIVE(func) \
    BC_B(0x0C), \
    BC_PTR(func)

// Adds a float to the specified field.
#define ADD_FLOAT(field, value) \
    BC_BBH(0x0D, field, value)

// Sets the specified field to a float.
#define SET_FLOAT(field, value) \
    BC_BBH(0x0E, field, value)

// Adds an integer to the specified field.
#define ADD_INT(field, value) \
    BC_BBH(0x0F, field, value)

// Sets the specified field to an integer.
#define SET_INT(field, value) \
    BC_BBH(0x10, field, value)

// Performs a bitwise OR with the specified field and the given integer.
// Usually used to set an object's flags.
#define OR_INT(field, value) \
    BC_BBH(0x11, field, value)

// Performs a bit clear with the specified short. Unused in favor of the 32-bit version.
#define BIT_CLEAR(field, value) \
    BC_BBH(0x12, field, value)

// TODO: this one needs a better name / labelling
// Gets a random short, right shifts it the specified amount and adds min to it, then sets the specified field to that value.
#define SET_INT_RAND_RSHIFT(field, min, rshift) \
    BC_BBH(0x13, field, min), \
    BC_H(rshift)

// Sets the specified field to a random float in the given range.
#define SET_RANDOM_FLOAT(field, min, range) \
    BC_BBH(0x14, field, min), \
    BC_H(range)

// Sets the specified field to a random integer in the given range.
#define SET_RANDOM_INT(field, min, range) \
    BC_BBH(0x15, field, min), \
    BC_H(range)

// Adds a random float in the given range to the specified field.
#define ADD_RANDOM_FLOAT(field, min, range) \
    BC_BBH(0x16, field, min), \
    BC_H(range)

// TODO: better name (unused anyway)
// Gets a random short, right shifts it the specified amount and adds min to it, then adds the value to the specified field. Unused.
#define ADD_INT_RAND_RSHIFT(field, min, rshift) \
    BC_BBH(0x17, field, min), \
    BC_H(rshift)

// No operation. Unused.
#define CMD_NOP_1(field) \
    BC_BB(0x18, field)

// No operation. Unused.
#define CMD_NOP_2(field) \
    BC_BB(0x19, field)

// No operation. Unused.
#define CMD_NOP_3(field) \
    BC_BB(0x1A, field)

// Sets the current model ID of the object.
#define SET_MODEL(modelID) \
    BC_B0H(0x1B, modelID)

// Spawns a child object with the specified model and behavior.
#define SPAWN_CHILD(modelID, behavior) \
    BC_B(0x1C), \
    BC_W(modelID), \
    BC_PTR(behavior)

// Exits the behavior script and despawns the object.
// Often used to end behavior scripts that do not contain an infinite loop.
#define DEACTIVATE() \
    BC_B(0x1D)

// Finds the floor triangle directly under the object and moves the object down to it.
#define DROP_TO_FLOOR() \
    BC_B(0x1E)

// Sets the destination float field to the sum of the values of the given float fields.
#define SUM_FLOAT(fieldDst, fieldSrc1, fieldSrc2) \
    BC_BBBB(0x1F, fieldDst, fieldSrc1, fieldSrc2)

// Sets the destination integer field to the sum of the values of the given integer fields. Unused.
#define SUM_INT(fieldDst, fieldSrc1, fieldSrc2) \
    BC_BBBB(0x20, fieldDst, fieldSrc1, fieldSrc2)

// Billboards the current object, making it always face the camera.
#define BILLBOARD() \
    BC_B(0x21)

#define CYLBOARD() \
    BC_B(0x38)

// Hides the current object.
#define HIDE() \
    BC_B(0x22)

// Sets the size of the object's cylindrical hitbox.
#define SET_HITBOX(radius, height) \
    BC_B(0x23), \
    BC_HH(radius, height)

// No operation. Unused.
#define CMD_NOP_4(field, value) \
    BC_BBH(0x24, field, value)

// Delays the behavior script for the number of frames given by the value of the specified field.
#define DELAY_VAR(field) \
    BC_BB(0x25, field)

// Unused. Marks the start of a loop that will repeat a certain number of times.
// Uses a u8 as the argument, instead of a s16 like the other version does.
#define BEGIN_REPEAT_UNUSED(count) \
    BC_BB(0x26, count)

// Loads the animations for the object. <field> is always set to oAnimations.
#define LOAD_ANIMATIONS(field, anims) \
    BC_BB(0x27, field), \
    BC_PTR(anims)

// Begins animation and sets the object's current animation index to the specified value.
#define ANIMATE(animIndex) \
    BC_BB(0x28, animIndex)

// Spawns a child object with the specified model and behavior, plus a behavior param.
#define SPAWN_CHILD_WITH_PARAM(bhvParam, modelID, behavior) \
    BC_B0H(0x29, bhvParam), \
    BC_W(modelID), \
    BC_PTR(behavior)

// Loads collision data for the object.
#define LOAD_COLLISION_DATA(collisionData) \
    BC_B(0x2A), \
    BC_PTR(collisionData)

// Sets the size of the object's cylindrical hitbox, and applies a downwards offset.
#define SET_HITBOX_WITH_OFFSET(radius, height, downOffset) \
    BC_B(0x2B), \
    BC_HH(radius, height), \
    BC_H(downOffset)

// Spawns a new object with the specified model and behavior.
#define SPAWN_OBJ(modelID, behavior) \
    BC_B(0x2C), \
    BC_W(modelID), \
    BC_PTR(behavior)

// Sets the home position of the object to its current position.
#define SET_HOME() \
    BC_B(0x2D)

// Sets the size of the object's cylindrical hurtbox.
#define SET_HURTBOX(radius, height) \
    BC_B(0x2E), \
    BC_HH(radius, height)

// Sets the object's interaction type.
#define SET_INTERACT_TYPE(type) \
    BC_B(0x2F), \
    BC_W(type)

// Sets various parameters that the object uses for calculating physics.
#define SET_OBJ_PHYSICS(wallHitboxRadius, gravity, bounciness, dragStrength, friction, buoyancy, unused1, unused2) \
    BC_B(0x30), \
    BC_HH(wallHitboxRadius, gravity), \
    BC_HH(bounciness, dragStrength), \
    BC_HH(friction, buoyancy), \
    BC_HH(unused1, unused2)

// Sets the object's interaction subtype. Unused.
#define SET_INTERACT_SUBTYPE(subtype) \
    BC_B(0x31), \
    BC_W(subtype)

// Sets the object's size to the specified percentage.
#define SCALE(unusedField, percent) \
    BC_BBH(0x32, unusedField, percent)

// Performs a bit clear on the object's parent's field with the specified value.
// Used for clearing active particle flags fron Mario's object.
#define PARENT_BIT_CLEAR(field, flags) \
    BC_BB(0x33, field), \
    BC_W(flags)

// Animates an object using texture animation. <field> is always set to oAnimState.
#define ANIMATE_TEXTURE(field, rate) \
    BC_BBH(0x34, field, rate)

// Disables rendering for the object.
#define DISABLE_RENDERING() \
    BC_B(0x35)

// Unused. Sets the specified field to an integer. Wastes 4 bytes of space for no reason at all.
#define SET_INT_UNUSED(field, value) \
    BC_BB(0x36, field), \
    BC_HH(0, value)

// Spawns a water droplet with the given parameters.
#define SPAWN_WATER_DROPLET(dropletParams) \
    BC_B(0x37), \
    BC_PTR(dropletParams)

// coop

// Defines the id of the behavior script
#define ID(id) \
    BC_B0H(0x39, id)

// Jumps to a new behavior command and stores the return address in the object's stack.
#define CALL_EXT(addr) \
    BC_B(0x3A), \
    BC_PTR(addr)

// Jumps to a new behavior script without saving anything.
#define GOTO_EXT(addr) \
    BC_B(0x3B), \
    BC_PTR(addr)

// Executes a native game function.
#define CALL_NATIVE_EXT(func) \
    BC_B(0x3C), \
    BC_PTR(func)

// Spawns a child object with the specified model and behavior.
#define SPAWN_CHILD_EXT(modelID, behavior) \
    BC_B(0x3D), \
    BC_W(modelID), \
    BC_PTR(behavior)

// Spawns a child object with the specified model and behavior, plus a behavior param.
#define SPAWN_CHILD_WITH_PARAM_EXT(bhvParam, modelID, behavior) \
    BC_B0H(0x3E, bhvParam), \
    BC_W(modelID), \
    BC_PTR(behavior)

// Spawns a new object with the specified model and behavior.
#define SPAWN_OBJ_EXT(modelID, behavior) \
    BC_B(0x3F), \
    BC_W(modelID), \
    BC_PTR(behavior)

// Loads the animations for the object. <field> is always set to oAnimations.
#define LOAD_ANIMATIONS_EXT(field, anims) \
    BC_BB(0x40, field), \
    BC_PTR(anims)

// Loads collision data for the object.
#define LOAD_COLLISION_DATA_EXT(collisionData) \
    BC_B(0x41), \
    BC_PTR(collisionData)

// This is a special case for behaviors hooked from LUA.
#define CALL_LUA_FUNC(func) \
    BC_B(0x42), \
    BC_W(func)

// OMM compatibility: BHV_* prefixed command macros.
// OMM computes field indices from struct offsets (rawData/ptrData),
// so its behaviors can be built without OBJECT_FIELDS_INDEX_DIRECTLY.
#define _OFFSETOF(x) ((uintptr_t) &(((struct Object *) NULL)->x))
#define _FIELD(field) ((_OFFSETOF(field) - (_OFFSETOF(field) < _OFFSETOF(ptrData) ? _OFFSETOF(rawData) : _OFFSETOF(ptrData))) / (_OFFSETOF(field) < _OFFSETOF(ptrData) ? sizeof(u32) : sizeof(uintptr_t)))

#define BHV_BEGIN(objList)                                                      BEGIN(objList)
#define BHV_DELAY(num)                                                          DELAY(num)
#define BHV_CALL(addr)                                                          CALL(addr)
#define BHV_RETURN()                                                            RETURN()
#define BHV_GOTO(addr)                                                          GOTO(addr)
#define BHV_BEGIN_REPEAT(count)                                                 BEGIN_REPEAT(count)
#define BHV_END_REPEAT()                                                        END_REPEAT()
#define BHV_END_REPEAT_CONTINUE()                                               END_REPEAT_CONTINUE()
#define BHV_BEGIN_LOOP()                                                        BEGIN_LOOP()
#define BHV_END_LOOP()                                                          END_LOOP()
#define BHV_BREAK()                                                             BREAK()
#define BHV_BREAK_UNUSED()                                                      BREAK_UNUSED()
#define BHV_CALL_NATIVE(func)                                                   CALL_NATIVE(func)
#define BHV_DEACTIVATE()                                                        DEACTIVATE()
#define BHV_DROP_TO_FLOOR()                                                     DROP_TO_FLOOR()
#define BHV_BILLBOARD()                                                         BILLBOARD()
#define BHV_CYLBOARD()                                                          CYLBOARD()
#define BHV_HIDE()                                                              HIDE()
#define BHV_SET_HOME()                                                          SET_HOME()
#define BHV_SET_MODEL(modelID)                                                  SET_MODEL(modelID)
#define BHV_SCALE(_0, percent)                                                  SCALE(_0, percent)
#define BHV_ANIMATE(animIndex)                                                  ANIMATE(animIndex)
#define BHV_ANIMATE_TEXTURE(field, rate)                                        ANIMATE_TEXTURE(_FIELD(field), rate)
#define BHV_DISABLE_RENDERING()                                                 DISABLE_RENDERING()
#define BHV_SPAWN_CHILD(modelID, behavior)                                      SPAWN_CHILD(modelID, behavior)
#define BHV_SPAWN_CHILD_WITH_PARAM(bhvParam, modelID, behavior)                 SPAWN_CHILD_WITH_PARAM(bhvParam, modelID, behavior)
#define BHV_SPAWN_OBJ(modelID, behavior)                                        SPAWN_OBJ(modelID, behavior)
#define BHV_SPAWN_WATER_DROPLET(dropletParams)                                  SPAWN_WATER_DROPLET(dropletParams)
#define BHV_SET_HITBOX(radius, height)                                          SET_HITBOX(radius, height)
#define BHV_SET_HITBOX_WITH_OFFSET(radius, height, downOffset)                  SET_HITBOX_WITH_OFFSET(radius, height, downOffset)
#define BHV_SET_HURTBOX(radius, height)                                         SET_HURTBOX(radius, height)
#define BHV_SET_INTERACT_TYPE(type)                                             SET_INTERACT_TYPE(type)
#define BHV_SET_INTERACT_SUBTYPE(subtype)                                       SET_INTERACT_SUBTYPE(subtype)
#define BHV_SET_OBJ_PHYSICS(radius, grav, bounce, drag, frict, buoy, _6, _7)    SET_OBJ_PHYSICS(radius, grav, bounce, drag, frict, buoy, _6, _7)
#define BHV_LOAD_ANIMATIONS(field, anims)                                       LOAD_ANIMATIONS(_FIELD(field), &(anims))
#define BHV_LOAD_COLLISION_DATA(collisionData)                                  LOAD_COLLISION_DATA(collisionData)
#define BHV_PARENT_BIT_CLEAR(field, flags)                                      PARENT_BIT_CLEAR(_FIELD(field), flags)

#define BHV_ADD_FLOAT(field, value)                                             ADD_FLOAT(_FIELD(field), value)
#define BHV_SET_FLOAT(field, value)                                             SET_FLOAT(_FIELD(field), value)
#define BHV_ADD_INT(field, value)                                               ADD_INT(_FIELD(field), value)
#define BHV_SET_INT(field, value)                                               SET_INT(_FIELD(field), value)
#define BHV_OR_INT(field, value)                                                OR_INT(_FIELD(field), value)
#define BHV_BIT_CLEAR(field, value)                                             BIT_CLEAR(_FIELD(field), value)
#define BHV_SET_INT_RAND_RSHIFT(field, min, rshift)                             SET_INT_RAND_RSHIFT(_FIELD(field), min, rshift)
#define BHV_SET_RANDOM_FLOAT(field, min, range)                                 SET_RANDOM_FLOAT(_FIELD(field), min, range)
#define BHV_SET_RANDOM_INT(field, min, range)                                   SET_RANDOM_INT(_FIELD(field), min, range)
#define BHV_ADD_RANDOM_FLOAT(field, min, range)                                 ADD_RANDOM_FLOAT(_FIELD(field), min, range)
#define BHV_ADD_INT_RAND_RSHIFT(field, min, rshift)                             ADD_INT_RAND_RSHIFT(_FIELD(field), min, rshift)
#define BHV_CMD_NOP_1(field)                                                    CMD_NOP_1(_FIELD(field))
#define BHV_CMD_NOP_2(field)                                                    CMD_NOP_2(_FIELD(field))
#define BHV_CMD_NOP_3(field)                                                    CMD_NOP_3(_FIELD(field))
#define BHV_CMD_NOP_4(field, value)                                             CMD_NOP_4(_FIELD(field), value)
#define BHV_DELAY_VAR(field)                                                    DELAY_VAR(_FIELD(field))
#define BHV_BEGIN_REPEAT_UNUSED(count)                                          BEGIN_REPEAT_UNUSED(count)
#define BHV_SUM_FLOAT(fieldDst, fieldSrc1, fieldSrc2)                           SUM_FLOAT(fieldDst, fieldSrc1, fieldSrc2)
#define BHV_SUM_INT(fieldDst, fieldSrc1, fieldSrc2)                             SUM_INT(fieldDst, fieldSrc1, fieldSrc2)
#define BHV_SET_INT_UNUSED(field, value)                                        SET_INT_UNUSED(_FIELD(field), value)

#endif // BEHAVIOR_COMMANDS_H