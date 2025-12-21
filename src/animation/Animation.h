#ifndef ANIMATION_H
#define ANIMATION_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

enum AnimationPreset {
    ENTRY_LEFT, ENTRY_RIGHT, ENTRY_TOP, ENTRY_BOTTOM, 
    EXIT_LEFT, EXIT_RIGHT, EXIT_TOP, EXIT_BOTTOM, 
    FADE_IN, FADE_OUT
};

enum AnimationSmoothing {
    LINEAR, EASY_IN, EASY_OUT, OVERSHOOT
};

class Animation
{
private:
    std::vector<vec2*> v2_targets;
    std::vector<vec2> v2_offsets;

    struct AnimationDetails {
        float duration = 0.f;
        bool to_target = false;
        bool save_transform = false;
        AnimationSmoothing smooth_fn_id = AnimationSmoothing::LINEAR;
    };

    struct V2AnimationComponent {
        vec2* variable; vec2 offset;
        vec2 start = vec2(0), end = vec2(0);
        AnimationDetails details;
    };
    struct FloatAnimationComponent {
        float* variable; float offset;
        float start = 0, end = 0;
        AnimationDetails details;
    };

    std::vector<V2AnimationComponent> v2_components;
    std::vector<FloatAnimationComponent> float_components;

    float full_duration = 0.f;
    bool active = false;
    float timer = 0.f;
    AnimationSmoothing smoothing_fn;

public:
    Animation() {}

    /* base functions */
    void update(float dt) {
        if (!active) return;

        for (auto& c : v2_components) {
            float progress = animation_smoothing(std::min(timer / c.details.duration,1.f), c.details.smooth_fn_id);
            vec2 value = c.start * (1-progress) + c.end * progress;
            c.variable->x = value.x;
            c.variable->y = value.y;
        }
        for (auto& c : float_components) {
            float progress = animation_smoothing(std::min(timer / c.details.duration,1.f), c.details.smooth_fn_id);
            float value = c.start * (1-progress) + c.end * progress;
            *c.variable = value;
        }
        
        timer += dt;
        if (timer >= full_duration) stop();

    }
    void start() {
        timer = 0.f;
        active = true;

        for (auto& c : v2_components) {
            c.start = c.details.to_target ? *c.variable + c.offset : *c.variable; 
            c.end = c.details.to_target ? *c.variable : *c.variable + c.offset; 
        }
        for (auto& c : float_components) {
            c.start = c.details.to_target ? *c.variable + c.offset : *c.variable; 
            c.end = c.details.to_target ? *c.variable : *c.variable + c.offset; 
        }
    }
    void stop() {
        for (auto& c : v2_components) { *c.variable = (c.details.to_target||c.details.save_transform) ? c.end : c.start; }
        for (auto& c : float_components) { *c.variable = (c.details.to_target||c.details.save_transform) ? c.end : c.start; }
        active = false;
    }

    /* animation componetns */
    void add_animation_component(vec2& target, vec2 offset, float duration_seconds, bool to_target = false, AnimationSmoothing smoothing = AnimationSmoothing::LINEAR, bool save_transform = false) {
        V2AnimationComponent component = { &target, offset };
        component.details.to_target = to_target;
        component.details.duration = duration_seconds;
        component.details.smooth_fn_id = smoothing;
        component.details.save_transform = save_transform;
        v2_components.push_back(component);
        full_duration = std::max(full_duration, duration_seconds);
    }
    void add_animation_component(float& target, float offset, float duration_seconds, bool to_target = false, AnimationSmoothing smoothing = AnimationSmoothing::LINEAR, bool save_transform = false) {
        FloatAnimationComponent component = { &target, offset };
        component.details.to_target = to_target;
        component.details.duration = duration_seconds;
        component.details.smooth_fn_id = smoothing;
        component.details.save_transform = save_transform;
        float_components.push_back(component);
        full_duration = std::max(full_duration, duration_seconds);
    }

    /* helpers */
    bool is_active() { return active; }

private:
    float animation_smoothing(float t, AnimationSmoothing smooth_fn) {
        switch(smooth_fn) {
            case AnimationSmoothing::LINEAR:
                return t;

            case AnimationSmoothing::EASY_IN:
                // Slow start, fast end (Quadratic)
                return t * t;

            case AnimationSmoothing::EASY_OUT:
                // Fast start, slow end (Quadratic)
                // Formula: 1 - (1 - t)^2
                return 1.0f - (1.0f - t) * (1.0f - t);

            case AnimationSmoothing::OVERSHOOT: {
                // Goes slightly past the target and "springs" back
                // s controls the amount of overshoot (1.70158 is the standard "Back" easing constant)
                const float s = 1.70158f;
                return (s + 1.0f) * (t - 1.0f) * (t - 1.0f) * (t - 1.0f) + (s * (t - 1.0f) * (t - 1.0f)) + 1.0f;
            }

            default:
                return t;
        }
    }
};

#endif 