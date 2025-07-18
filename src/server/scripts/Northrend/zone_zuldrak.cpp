/*
 * This file is part of the AzerothCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "CreatureScript.h"
#include "GameObjectScript.h"
#include "PassiveAI.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "SpellScript.h"
#include "SpellScriptLoader.h"
#include "Vehicle.h"

enum AlchemistItemRequirements
{
    QUEST_ALCHEMIST_APPRENTICE      = 12541,
    NPC_FINKLESTEIN                 = 28205,
};

const uint32 AA_ITEM_ENTRY[24] = {38336, 39669, 38342, 38340, 38344, 38369, 38396, 38398, 38338, 38386, 38341, 38384, 38397, 38381, 38337, 38393, 38339, 39668, 39670, 38346, 38379, 38345, 38343, 38370};
const uint32 AA_AURA_ID[24]    = {51095, 53153, 51100, 51087, 51091, 51081, 51072, 51079, 51018, 51067, 51055, 51064, 51077, 51062, 51057, 51069, 51059, 53150, 53158, 51093, 51097, 51102, 51083, 51085};
const char*  AA_ITEM_NAME[24]  = {"Crystallized Hogsnot", "Ghoul Drool", "Trollbane", "Amberseed", "Shrunken Dragon's Claw",
                                  "Wasp's Wings", "Hairy Herring Head", "Icecrown Bottled Water", "Knotroot", "Muddy Mire Maggot", "Pickled Eagle Egg",
                                  "Pulverized Gargoyle Teeth", "Putrid Pirate Perspiration", "Seasoned Slider Cider", "Speckled Guano", "Spiky Spider Egg",
                                  "Withered Batwing", "Abomination Guts", "Blight Crystal", "Chilled Serpent Mucus", "Crushed Basilisk Crystals",
                                  "Frozen Spider Ichor", "Prismatic Mojo", "Raptor Claw"
                                 };

class npc_finklestein : public CreatureScript
{
public:
    npc_finklestein() : CreatureScript("npc_finklestein") { }

    struct npc_finklesteinAI : public ScriptedAI
    {
        npc_finklesteinAI(Creature* creature) : ScriptedAI(creature) {}

        std::map<ObjectGuid, uint32> questList;

        void ClearPlayerOnTask(ObjectGuid guid)
        {
            std::map<ObjectGuid, uint32>::iterator itr = questList.find(guid);
            if (itr != questList.end())
                questList.erase(itr);
        }

        bool IsPlayerOnTask(ObjectGuid guid)
        {
            std::map<ObjectGuid, uint32>::const_iterator itr = questList.find(guid);
            return itr != questList.end();
        }

        void RightClickCauldron(ObjectGuid guid)
        {
            if (questList.empty())
                return;

            std::map<ObjectGuid, uint32>::iterator itr = questList.find(guid);
            if (itr == questList.end())
                return;

            Player* player = ObjectAccessor::GetPlayer(*me, guid);
            if (player)
            {
                uint32 itemCode = itr->second;

                uint32 itemEntry = GetTaskItemEntry(itemCode);
                uint32 auraId = GetTaskAura(itemCode);
                uint32 counter = GetTaskCounter(itemCode);
                if (player->HasAura(auraId))
                {
                    // player still has aura, but no item. Skip
                    if (!player->HasItemCount(itemEntry))
                        return;

                    // if we are here, all is ok (aura and item present)
                    player->DestroyItemCount(itemEntry, 1, true);
                    player->RemoveAurasDueToSpell(auraId);

                    if (counter < 6)
                    {
                        StartNextTask(player->GetGUID(), counter + 1);
                        return;
                    }
                    else
                        player->KilledMonsterCredit(28248);
                }
                else
                {
                    // if we are here, it means we failed :(
                    player->SetQuestStatus(QUEST_ALCHEMIST_APPRENTICE, QUEST_STATUS_FAILED);
                }
            }
            questList.erase(itr);
        }

        // Generate a Task and announce it to the player
        void StartNextTask(ObjectGuid playerGUID, uint32 counter)
        {
            if (counter > 6)
                return;

            Player* player = ObjectAccessor::GetPlayer(*me, playerGUID);
            if (!player)
                return;

            // Generate Item Code
            uint32 itemCode = SelectRandomCode(counter);
            questList[playerGUID] = itemCode;

            // Decode Item Entry, Get Item Name, Generate Emotes
            //uint32 itemEntry = GetTaskItemEntry(itemCode);
            uint32 auraId = GetTaskAura(itemCode);
            const char* itemName = GetTaskItemName(itemCode);

            switch (counter)
            {
                case 1:
                    me->TextEmote("Quickly, get me some...", player, true);
                    me->TextEmote(itemName, player, true);
                    me->CastSpell(player, auraId, true);
                    break;
                case 2:
                    me->TextEmote("Find me some...", player, true);
                    me->TextEmote(itemName, player, true);
                    me->CastSpell(player, auraId, true);
                    break;
                case 3:
                    me->TextEmote("I think it needs...", player, true);
                    me->TextEmote(itemName, player, true);
                    me->CastSpell(player, auraId, true);
                    break;
                case 4:
                    me->TextEmote("Alright, now fetch me some...", player, true);
                    me->TextEmote(itemName, player, true);
                    me->CastSpell(player, auraId, true);
                    break;
                case 5:
                    me->TextEmote("Before it thickens, we must add...", player, true);
                    me->TextEmote(itemName, player, true);
                    me->CastSpell(player, auraId, true);
                    break;
                case 6:
                    me->TextEmote("It's thickening! Quickly get me some...", player, true);
                    me->TextEmote(itemName, player, true);
                    me->CastSpell(player, auraId, true);
                    break;
            }
        }

        uint32 SelectRandomCode(uint32 counter)  { return (counter * 100 + urand(0, 23)); }

        uint32 GetTaskCounter(uint32 itemcode)   { return itemcode / 100; }
        uint32 GetTaskAura(uint32 itemcode)      { return AA_AURA_ID[itemcode % 100]; }
        uint32 GetTaskItemEntry(uint32 itemcode) { return AA_ITEM_ENTRY[itemcode % 100]; }
        const char* GetTaskItemName(uint32 itemcode)  { return AA_ITEM_NAME[itemcode % 100]; }
    };

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest) override
    {
        if (quest->GetQuestId() == QUEST_ALCHEMIST_APPRENTICE)
            if (creature->AI() && CAST_AI(npc_finklestein::npc_finklesteinAI, creature->AI()))
                CAST_AI(npc_finklestein::npc_finklesteinAI, creature->AI())->ClearPlayerOnTask(player->GetGUID());

        return true;
    }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        if (creature->IsQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        SendGossipMenuFor(player, player->GetGossipTextId(creature), creature->GetGUID());

        if (player->GetQuestStatus(QUEST_ALCHEMIST_APPRENTICE) == QUEST_STATUS_INCOMPLETE)
        {
            if (creature->AI() && CAST_AI(npc_finklestein::npc_finklesteinAI, creature->AI()))
                if (!CAST_AI(npc_finklestein::npc_finklesteinAI, creature->AI())->IsPlayerOnTask(player->GetGUID()))
                    AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I'm ready to begin. What is the first ingredient you require?", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

            SendGossipMenuFor(player, player->GetGossipTextId(creature), creature->GetGUID());
        }

        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*uiSender*/, uint32 uiAction) override
    {
        CloseGossipMenuFor(player);
        if (uiAction == GOSSIP_ACTION_INFO_DEF + 1)
        {
            CloseGossipMenuFor(player);
            if (creature->AI() && CAST_AI(npc_finklestein::npc_finklesteinAI, creature->AI()))
                CAST_AI(npc_finklestein::npc_finklesteinAI, creature->AI())->StartNextTask(player->GetGUID(), 1);
        }

        return true;
    }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_finklesteinAI(creature);
    }
};

class go_finklestein_cauldron : public GameObjectScript
{
public:
    go_finklestein_cauldron() : GameObjectScript("go_finklestein_cauldron") { }

    bool OnGossipHello(Player* player, GameObject* go) override
    {
        Creature* finklestein = go->FindNearestCreature(NPC_FINKLESTEIN, 30.0f, true);
        if (finklestein && finklestein->AI())
            CAST_AI(npc_finklestein::npc_finklesteinAI, finklestein->AI())->RightClickCauldron(player->GetGUID());

        return true;
    }
};

enum overlordDrakuru
{
    SPELL_SHADOW_BOLT                   = 54113,
    SPELL_SCOURGE_DISGUISE_EXPIRING     = 52010,
    SPELL_THROW_BRIGHT_CRYSTAL          = 54087,
    SPELL_TELEPORT_EFFECT               = 52096,
    SPELL_SCOURGE_DISGUISE              = 51966,
    SPELL_SCOURGE_DISGUISE_INSTANT_CAST = 52192,
    SPELL_BLIGHT_FOG                    = 54104,
    SPELL_THROW_PORTAL_CRYSTAL          = 54209,
    SPELL_ARTHAS_PORTAL                 = 51807,
    SPELL_TOUCH_OF_DEATH                = 54236,
    SPELL_DRAKURU_DEATH                 = 54248,
    SPELL_SUMMON_SKULL                  = 54253,

    QUEST_BETRAYAL                      = 12713,

    NPC_BLIGHTBLOOD_TROLL               = 28931,
    NPC_LICH_KING                       = 28498,

    EVENT_BETRAYAL_1                    = 1,
    EVENT_BETRAYAL_2                    = 2,
    EVENT_BETRAYAL_3                    = 3,
    EVENT_BETRAYAL_4                    = 4,
    EVENT_BETRAYAL_5                    = 5,
    EVENT_BETRAYAL_6                    = 6,
    EVENT_BETRAYAL_7                    = 7,
    EVENT_BETRAYAL_8                    = 8,
    EVENT_BETRAYAL_9                    = 9,
    EVENT_BETRAYAL_10                   = 10,
    EVENT_BETRAYAL_11                   = 11,
    EVENT_BETRAYAL_12                   = 12,
    EVENT_BETRAYAL_13                   = 13,
    EVENT_BETRAYAL_14                   = 14,
    EVENT_BETRAYAL_SHADOW_BOLT          = 20,
    EVENT_BETRAYAL_CRYSTAL              = 21,
    EVENT_BETRAYAL_COMBAT_TALK          = 22,

    SAY_DRAKURU_0                       = 0,
    SAY_DRAKURU_1                       = 1,
    SAY_DRAKURU_2                       = 2,
    SAY_DRAKURU_3                       = 3,
    SAY_DRAKURU_4                       = 4,
    SAY_DRAKURU_5                       = 5,
    SAY_DRAKURU_6                       = 6,
    SAY_DRAKURU_7                       = 7,
    SAY_LICH_7                          = 7,
    SAY_LICH_8                          = 8,
    SAY_LICH_9                          = 9,
    SAY_LICH_10                         = 10,
    SAY_LICH_11                         = 11,
    SAY_LICH_12                         = 12,
};

class npc_overlord_drakuru_betrayal : public CreatureScript
{
public:
    npc_overlord_drakuru_betrayal() : CreatureScript("npc_overlord_drakuru_betrayal") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_overlord_drakuru_betrayalAI(creature);
    }

    struct npc_overlord_drakuru_betrayalAI : public ScriptedAI
    {
        npc_overlord_drakuru_betrayalAI(Creature* creature) : ScriptedAI(creature), summons(me)
        {
        }

        EventMap events;
        SummonList summons;
        ObjectGuid playerGUID;
        ObjectGuid lichGUID;

        void EnterEvadeMode(EvadeReason why) override
        {
            if (playerGUID)
                if (Player* player = ObjectAccessor::GetPlayer(*me, playerGUID))
                    if (player->IsWithinDistInMap(me, 80))
                        return;
            me->SetFaction(FACTION_UNDEAD_SCOURGE);
            me->RemoveUnitFlag(UNIT_FLAG_NON_ATTACKABLE);
            ScriptedAI::EnterEvadeMode(why);
        }

        void Reset() override
        {
            events.Reset();
            summons.DespawnAll();
            playerGUID.Clear();
            lichGUID.Clear();
            me->SetFaction(FACTION_UNDEAD_SCOURGE);
            me->SetVisible(false);
            me->RemoveUnitFlag(UNIT_FLAG_NON_ATTACKABLE);
        }

        void MoveInLineOfSight(Unit* who) override
        {
            if (who->IsPlayer())
            {
                if (playerGUID)
                {
                    if (who->GetGUID() != playerGUID)
                    {
                        Player* player = ObjectAccessor::GetPlayer(*me, playerGUID);
                        if (player && player->IsWithinDistInMap(me, 80))
                            who->ToPlayer()->NearTeleportTo(6143.76f, -1969.7f, 417.57f, 2.08f);
                        else
                        {
                            EnterEvadeMode(EVADE_REASON_OTHER);
                            return;
                        }
                    }
                    else
                        ScriptedAI::MoveInLineOfSight(who);
                }
                else if (who->ToPlayer()->GetQuestStatus(QUEST_BETRAYAL) == QUEST_STATUS_INCOMPLETE && who->HasAura(SPELL_SCOURGE_DISGUISE))
                {
                    me->SetVisible(true);
                    playerGUID = who->GetGUID();
                    events.ScheduleEvent(EVENT_BETRAYAL_1, 5s);
                }
            }
            else
                ScriptedAI::MoveInLineOfSight(who);
        }

        void JustSummoned(Creature* cr) override
        {
            summons.Summon(cr);
            if (cr->GetEntry() == NPC_BLIGHTBLOOD_TROLL)
                cr->CastSpell(cr, SPELL_TELEPORT_EFFECT, true);
            else
            {
                me->SetFacingToObject(cr);
                lichGUID = cr->GetGUID();
                float o = me->GetAngle(cr);
                cr->GetMotionMaster()->MovePoint(0, me->GetPositionX() + cos(o) * 6.0f, me->GetPositionY() + std::sin(o) * 6.0f, me->GetPositionZ());
            }
        }

        void JustEngagedWith(Unit*) override
        {
            Talk(SAY_DRAKURU_3);
            events.ScheduleEvent(EVENT_BETRAYAL_SHADOW_BOLT, 2s);
            events.ScheduleEvent(EVENT_BETRAYAL_CRYSTAL, 5s);
            events.ScheduleEvent(EVENT_BETRAYAL_COMBAT_TALK, 20s);
        }

        void DamageTaken(Unit*, uint32& damage, DamageEffectType, SpellSchoolMask) override
        {
            if (damage >= me->GetHealth() && !me->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE))
            {
                damage = 0;
                me->RemoveAllAuras();
                me->CombatStop();
                me->SetUnitFlag(UNIT_FLAG_NON_ATTACKABLE);
                me->SetFaction(FACTION_FRIENDLY);
                events.Reset();
                events.ScheduleEvent(EVENT_BETRAYAL_4, 1s);
            }
        }

        void SpellHitTarget(Unit* target, SpellInfo const* spellInfo) override
        {
            if (spellInfo->Id == SPELL_THROW_PORTAL_CRYSTAL)
                if (Aura* aura = target->AddAura(SPELL_ARTHAS_PORTAL, target))
                    aura->SetDuration(48000);
        }

        void SpellHit(Unit*  /*caster*/, SpellInfo const* spellInfo) override
        {
            if (spellInfo->Id == SPELL_TOUCH_OF_DEATH)
            {
                me->CastSpell(me, SPELL_DRAKURU_DEATH, true);
                me->CastSpell(me, SPELL_SUMMON_SKULL, true);
            }
        }

        void UpdateAI(uint32 diff) override
        {
            events.Update(diff);
            switch (events.ExecuteEvent())
            {
                case EVENT_BETRAYAL_1:
                    Talk(SAY_DRAKURU_0);
                    events.ScheduleEvent(EVENT_BETRAYAL_2, 5s);
                    break;
                case EVENT_BETRAYAL_2:
                    me->SummonCreature(NPC_BLIGHTBLOOD_TROLL, 6184.1f, -1969.9f, 586.76f, 4.5f);
                    me->SummonCreature(NPC_BLIGHTBLOOD_TROLL, 6222.9f, -2026.5f, 586.76f, 2.9f);
                    me->SummonCreature(NPC_BLIGHTBLOOD_TROLL, 6166.2f, -2065.4f, 586.76f, 1.4f);
                    me->SummonCreature(NPC_BLIGHTBLOOD_TROLL, 6127.5f, -2008.7f, 586.76f, 0.0f);
                    events.ScheduleEvent(EVENT_BETRAYAL_3, 5s);
                    break;
                case EVENT_BETRAYAL_3:
                    Talk(SAY_DRAKURU_1);
                    Talk(SAY_DRAKURU_2);
                    if (Player* player = ObjectAccessor::GetPlayer(*me, playerGUID))
                        player->CastSpell(player, SPELL_SCOURGE_DISGUISE_EXPIRING, true);
                    if (Aura* aur = me->AddAura(SPELL_BLIGHT_FOG, me))
                        aur->SetDuration(22000);
                    break;
                case EVENT_BETRAYAL_4:
                    Talk(SAY_DRAKURU_5);
                    events.ScheduleEvent(EVENT_BETRAYAL_5, 6s);
                    break;
                case EVENT_BETRAYAL_5:
                    Talk(SAY_DRAKURU_6);
                    me->CastSpell(me, SPELL_THROW_PORTAL_CRYSTAL, true);
                    events.ScheduleEvent(EVENT_BETRAYAL_6, 8s);
                    break;
                case EVENT_BETRAYAL_6:
                    me->SummonCreature(NPC_LICH_KING, 6142.9f, -2011.6f, 590.86f, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 41000);
                    events.ScheduleEvent(EVENT_BETRAYAL_7, 8s);
                    break;
                case EVENT_BETRAYAL_7:
                    Talk(SAY_DRAKURU_7);
                    events.ScheduleEvent(EVENT_BETRAYAL_8, 5s);
                    break;
                case EVENT_BETRAYAL_8:
                    if (Creature* lich = ObjectAccessor::GetCreature(*me, lichGUID))
                        lich->AI()->Talk(SAY_LICH_7);
                    events.ScheduleEvent(EVENT_BETRAYAL_9, 6s);
                    break;
                case EVENT_BETRAYAL_9:
                    if (Creature* lich = ObjectAccessor::GetCreature(*me, lichGUID))
                    {
                        lich->AI()->Talk(SAY_LICH_8);
                        lich->CastSpell(me, SPELL_TOUCH_OF_DEATH, false);
                    }
                    events.ScheduleEvent(EVENT_BETRAYAL_10, 4s);
                    break;
                case EVENT_BETRAYAL_10:
                    me->SetVisible(false);
                    if (Creature* lich = ObjectAccessor::GetCreature(*me, lichGUID))
                        lich->AI()->Talk(SAY_LICH_9);
                    events.ScheduleEvent(EVENT_BETRAYAL_11, 4s);
                    break;
                case EVENT_BETRAYAL_11:
                    if (Creature* lich = ObjectAccessor::GetCreature(*me, lichGUID))
                        lich->AI()->Talk(SAY_LICH_10);
                    events.ScheduleEvent(EVENT_BETRAYAL_12, 6s);
                    break;
                case EVENT_BETRAYAL_12:
                    if (Creature* lich = ObjectAccessor::GetCreature(*me, lichGUID))
                        lich->AI()->Talk(SAY_LICH_11);
                    events.ScheduleEvent(EVENT_BETRAYAL_13, 3s);
                    break;
                case EVENT_BETRAYAL_13:
                    if (Creature* lich = ObjectAccessor::GetCreature(*me, lichGUID))
                    {
                        lich->AI()->Talk(SAY_LICH_12);
                        lich->GetMotionMaster()->MovePoint(0, 6143.8f, -2011.5f, 590.9f);
                    }
                    events.ScheduleEvent(EVENT_BETRAYAL_14, 7s);
                    break;
                case EVENT_BETRAYAL_14:
                    playerGUID.Clear();
                    EnterEvadeMode(EVADE_REASON_OTHER);
                    break;
            }

            if (me->GetFaction() == FACTION_FRIENDLY || me->HasUnitState(UNIT_STATE_CASTING | UNIT_STATE_STUNNED))
                return;

            if (!UpdateVictim())
                return;

            switch (events.ExecuteEvent())
            {
                case EVENT_BETRAYAL_SHADOW_BOLT:
                    if (!me->IsWithinMeleeRange(me->GetVictim()))
                        me->CastSpell(me->GetVictim(), SPELL_SHADOW_BOLT, false);
                    events.Repeat(2s);
                    break;
                case EVENT_BETRAYAL_CRYSTAL:
                    if (Player* player = ObjectAccessor::GetPlayer(*me, playerGUID))
                        me->CastSpell(player, SPELL_THROW_BRIGHT_CRYSTAL, true);
                    events.Repeat(6s, 15s);
                    break;
                case EVENT_BETRAYAL_COMBAT_TALK:
                    Talk(SAY_DRAKURU_4);
                    events.Repeat(20s);
                    break;
            }

            DoMeleeAttackIfReady();
        }
    };
};

/*####
## npc_drakuru_shackles
####*/

enum DrakuruShackles
{
    NPC_RAGECLAW                             = 29686,
    QUEST_TROLLS_IS_GONE_CRAZY               = 12861,
    SPELL_LEFT_CHAIN                         = 59951,
    SPELL_RIGHT_CHAIN                        = 59952,
    SPELL_UNLOCK_SHACKLE                     = 55083,
    SPELL_FREE_RAGECLAW                      = 55223
};

class npc_drakuru_shackles : public CreatureScript
{
public:
    npc_drakuru_shackles() : CreatureScript("npc_drakuru_shackles") { }

    struct npc_drakuru_shacklesAI : public NullCreatureAI
    {
        npc_drakuru_shacklesAI(Creature* creature) : NullCreatureAI(creature)
        {
            _rageclawGUID.Clear();
            timer = 0;
        }

        void Reset() override
        {
            me->SetUnitFlag(UNIT_FLAG_NOT_SELECTABLE);
        }

        void UpdateAI(uint32 diff) override
        {
            timer += diff;
            if (timer >= 2000)
            {
                timer = 0;
                if (_rageclawGUID)
                    return;

                if (Creature* cr = me->FindNearestCreature(NPC_RAGECLAW, 10.0f))
                {
                    _rageclawGUID = cr->GetGUID();
                    LockRageclaw(cr);
                }
            }
        }

        void LockRageclaw(Creature* rageclaw)
        {
            // pointer check not needed
            me->SetFacingToObject(rageclaw);
            rageclaw->SetFacingToObject(me);

            DoCast(rageclaw, SPELL_LEFT_CHAIN, true);
            DoCast(rageclaw, SPELL_RIGHT_CHAIN, true);
        }

        void UnlockRageclaw(Unit*  /*who*/, Creature* rageclaw)
        {
            // pointer check not needed
            DoCast(rageclaw, SPELL_FREE_RAGECLAW, true);
            _rageclawGUID.Clear();
            me->DespawnOrUnsummon(1);
        }

        void SpellHit(Unit* caster, SpellInfo const* spell) override
        {
            if (spell->Id == SPELL_UNLOCK_SHACKLE)
            {
                if (caster->ToPlayer()->GetQuestStatus(QUEST_TROLLS_IS_GONE_CRAZY) == QUEST_STATUS_INCOMPLETE)
                {
                    if (Creature* rageclaw = ObjectAccessor::GetCreature(*me, _rageclawGUID))
                    {
                        UnlockRageclaw(caster, rageclaw);
                        caster->ToPlayer()->KilledMonster(rageclaw->GetCreatureTemplate(), _rageclawGUID);
                        me->DespawnOrUnsummon();
                    }
                    else
                        me->setDeathState(DeathState::JustDied);
                }
            }
        }

    private:
        ObjectGuid _rageclawGUID;
        uint32 timer;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_drakuru_shacklesAI(creature);
    }
};

/*####
## npc_captured_rageclaw
####*/

enum Rageclaw
{
    SPELL_UNSHACKLED                         = 55085,
    SPELL_KNEEL                              = 39656,
    SAY_RAGECLAW                             = 0
};

class npc_captured_rageclaw : public CreatureScript
{
public:
    npc_captured_rageclaw() : CreatureScript("npc_captured_rageclaw") { }

    struct npc_captured_rageclawAI : public NullCreatureAI
    {
        npc_captured_rageclawAI(Creature* creature) : NullCreatureAI(creature) { }

        void Reset() override
        {
            me->SetFaction(FACTION_FRIENDLY);
            DoCast(me, SPELL_KNEEL, true); // Little Hack for kneel - Thanks Illy :P
        }

        void SpellHit(Unit* /*caster*/, SpellInfo const* spell) override
        {
            if (spell->Id == SPELL_FREE_RAGECLAW)
            {
                me->RemoveAurasDueToSpell(SPELL_LEFT_CHAIN);
                me->RemoveAurasDueToSpell(SPELL_RIGHT_CHAIN);
                me->RemoveAurasDueToSpell(SPELL_KNEEL);
                me->SetFaction(me->GetCreatureTemplate()->faction);
                DoCast(me, SPELL_UNSHACKLED, true);
                Talk(SAY_RAGECLAW);
                me->GetMotionMaster()->MoveRandom(10);
                me->DespawnOrUnsummon(10000);
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_captured_rageclawAI(creature);
    }
};

/*####
## npc_released_offspring_harkoa
####*/

class npc_released_offspring_harkoa : public CreatureScript
{
public:
    npc_released_offspring_harkoa() : CreatureScript("npc_released_offspring_harkoa") { }

    struct npc_released_offspring_harkoaAI : public ScriptedAI
    {
        npc_released_offspring_harkoaAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset() override
        {
            float x, y, z;
            me->GetClosePoint(x, y, z, me->GetObjectSize() / 3, 25.0f);
            me->GetMotionMaster()->MovePoint(0, x, y, z);
        }

        void MovementInform(uint32 Type, uint32 /*uiId*/) override
        {
            if (Type != POINT_MOTION_TYPE)
                return;
            me->DespawnOrUnsummon();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_released_offspring_harkoaAI(creature);
    }
};

/*######
## npc_crusade_recruit
######*/

enum CrusadeRecruit
{
    SPELL_QUEST_CREDIT                       = 50633,
    QUEST_TROLL_PATROL_INTESTINAL_FORTITUDE  = 12509,
    SAY_RECRUIT                              = 0
};

enum CrusadeRecruitEvents
{
    EVENT_RECRUIT_1                          = 1,
    EVENT_RECRUIT_2                          = 2
};

class npc_crusade_recruit : public CreatureScript
{
public:
    npc_crusade_recruit() : CreatureScript("npc_crusade_recruit") { }

    struct npc_crusade_recruitAI : public ScriptedAI
    {
        npc_crusade_recruitAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset() override
        {
            me->SetNpcFlag(UNIT_NPC_FLAG_GOSSIP);
            me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_COWER);
            _heading = me->GetOrientation();
        }

        void UpdateAI(uint32 diff) override
        {
            _events.Update(diff);

            while (uint32 eventId = _events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_RECRUIT_1:
                        me->RemoveNpcFlag(UNIT_NPC_FLAG_GOSSIP);
                        me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_ONESHOT_NONE);
                        Talk(SAY_RECRUIT);
                        _events.ScheduleEvent(EVENT_RECRUIT_2, 3s);
                        break;
                    case EVENT_RECRUIT_2:
                        me->SetWalk(true);
                        me->GetMotionMaster()->MovePoint(0, me->GetPositionX() + (cos(_heading) * 10), me->GetPositionY() + (std::sin(_heading) * 10), me->GetPositionZ());
                        me->DespawnOrUnsummon(5000);
                        break;
                    default:
                        break;
                }
            }

            if (!UpdateVictim())
                return;
        }

        void sGossipSelect(Player* player, uint32 /*sender*/, uint32 /*action*/) override
        {
            _events.ScheduleEvent(EVENT_RECRUIT_1, 100ms);
            CloseGossipMenuFor(player);
            me->CastSpell(player, SPELL_QUEST_CREDIT, true);
            me->SetFacingToObject(player);
        }

    private:
        EventMap _events;
        float    _heading; // Store creature heading
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_crusade_recruitAI(creature);
    }
};

/*######
## Quest 12916: Our Only Hope!
## go_scourge_enclosure
######*/

enum ScourgeEnclosure
{
    QUEST_OUR_ONLY_HOPE                      = 12916,
    NPC_GYMER_DUMMY                          = 29928, // From quest template
    SPELL_GYMER_LOCK_EXPLOSION               = 55529
};

class go_scourge_enclosure : public GameObjectScript
{
public:
    go_scourge_enclosure() : GameObjectScript("go_scourge_enclosure") { }

    bool OnGossipHello(Player* player, GameObject* go) override
    {
        go->UseDoorOrButton();
        if (player->GetQuestStatus(QUEST_OUR_ONLY_HOPE) == QUEST_STATUS_INCOMPLETE)
        {
            Creature* gymerDummy = go->FindNearestCreature(NPC_GYMER_DUMMY, 20.0f);
            if (gymerDummy)
            {
                player->KilledMonsterCredit(gymerDummy->GetEntry(), gymerDummy->GetGUID());
                gymerDummy->CastSpell(gymerDummy, SPELL_GYMER_LOCK_EXPLOSION, true);
                gymerDummy->DespawnOrUnsummon();
            }
        }
        return true;
    }
};

enum ScourgeDisguiseInstability
{
    SCOURGE_DISGUISE_FAILING_MESSAGE_1       = 28552, // Scourge Disguise Failing! Find a safe place!
    SCOURGE_DISGUISE_FAILING_MESSAGE_2       = 28758, // Scourge Disguise Failing! Run for cover!
    SCOURGE_DISGUISE_FAILING_MESSAGE_3       = 28759, // Scourge Disguise Failing! Hide quickly!
};
std::vector<uint32> const scourgeDisguiseTextIDs = { SCOURGE_DISGUISE_FAILING_MESSAGE_1, SCOURGE_DISGUISE_FAILING_MESSAGE_2, SCOURGE_DISGUISE_FAILING_MESSAGE_3 };

class spell_scourge_disguise_instability : public AuraScript
{
    PrepareAuraScript(spell_scourge_disguise_instability);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SCOURGE_DISGUISE_EXPIRING });
    }

    void HandleApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        SetDuration(urand(3 * MINUTE * IN_MILLISECONDS, 5 * MINUTE * IN_MILLISECONDS));
    }

    void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (Unit* caster = GetCaster())
        {
            if (Player* player = caster->ToPlayer())
            {
                if (player->HasAnyAuras(SPELL_SCOURGE_DISGUISE, SPELL_SCOURGE_DISGUISE_INSTANT_CAST))
                {
                    uint32 textId = Acore::Containers::SelectRandomContainerElement(scourgeDisguiseTextIDs);
                    player->Unit::Whisper(textId, player, true);
                    player->CastSpell(player, SPELL_SCOURGE_DISGUISE_EXPIRING, true);
                }
            }
        }
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_scourge_disguise_instability::HandleApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        OnEffectRemove += AuraEffectRemoveFn(spell_scourge_disguise_instability::HandleRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

void AddSC_zuldrak()
{
    new npc_finklestein();
    new go_finklestein_cauldron();
    new npc_overlord_drakuru_betrayal();
    new npc_drakuru_shackles();
    new npc_captured_rageclaw();
    new npc_released_offspring_harkoa();
    new npc_crusade_recruit();
    new go_scourge_enclosure();

    RegisterSpellScript(spell_scourge_disguise_instability);
}
